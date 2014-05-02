
#include "sharedmemmanager.h"

/* Array of all light locks in the shared memory. */
ULightLockPadded* lightLockArray = NULL;
int	              heldLocksNum   = 0;

/* Array of held locks. Since light locks are applied 
 * only for a short period of time, it is not likely 
 * that there will be too many locks held simultaneously.
 */
ELightLockMode    heldLightLocks[MAX_SIMULTANEOUS_LOCKS];

/* Calculate number of light locks needed */
int lightLocksNumber()
{ 
    int	  locksNum = 0;  

    /* Set up the initial locks number. */
	locksNum = (int)LL_LightLocksFixedNum;

	/* buffer manager needs 2 locks for each shared buffer. */
	locksNum += 2 * bufNum;

	locksNum += MaxBackends + NUM_AUXILIARY_PROCS;

	return numLocks;  
}

size_t lightLocksSize(
    void*           self)
{
	ILightLockManager  llm = (ILightLockManager)self;
	ISharedMemManager  smm = (ISharedMemManager)llm->sharedMemManager; 

	size_t   size; 
	int      locksNum = lightLocksNumber();
    
	/* Compute space for LightLocks array */
	size = smm->sizeMultiply(smm, locksNum, sizeof(ULightLockPadded));

	/* Add room for alignment */
	size = smm->addSize(smm, size, 2 * sizeof(int) + LIGHT_LOCK_PADDED_SIZE);

	return size;
}

void initLightLockArray(
	void*           self)
{
    ILightLockManager  llm = (ILightLockManager)self;
	ISharedMemManager  smm = (ISharedMemManager)llm->sharedMemManager; 

    int                numLocks  = lightLocksNumber();
    size_t             sizeLocks = lightLocksSize(self);
	char*              memPointer; 
	ULightLockPadded   lock;
	int*               lockCounter;
	int                i;

	/* Allocate space from shared memory for locks array */
	memPointer = (char*)smm->allocSharedMem(smm, sizeLocks);

	/* Add space for dynamic allocation counter. */
    memPointer += 2 * sizeof(int);

	/* Align light locks array. */
    memPointer += LIGHT_LOCK_PADDED_SIZE - ((uint)memPointer) % LIGHT_LOCK_PADDED_SIZE;

    lightLockArray = (ULightLockPadded*)memPointer;  

	for (i = 0, lock = lightLockArray; i < numLocks; i++, lock++)
	{
		lock->lock.mutex            = 0;
		lock->lock.releaseOK        = True;
		lock->lock.exclHoldersNum   = 0;
		lock->lock.sharedHoldersNum = 0;
		lock->lock.head             = NULL;
		lock->lock.tail             = NULL;
	}

    lockCounter    = (int*)((char*)lightLockArray - 2 * sizeof(int));
    lockCounter[0] = LL_LightLocksFixedNum;
	lockCounter[1] = numLocks;
}

void lightLockAcquire(
    void*                 self,
	ELightLockType        type, 
	ELightLockMode        mode)
{
    ILightLockManager     llm   = (ILightLockManager)self;
	ISharedMemManager     smm   = (ISharedMemManager)llm->sharedMemManager; 
	IErrorLogger          elog  = (IErrorLogger)llm->errorLogger;
    ISpinLockManager      slm   = (ISpinLockManager)llm->spinLockManager;
	ISemaphoreLockManager semm  = (ISemaphoreLockManager)llm->semLockManager;

    volatile LightLock   lock     = &(lightLockArray[type].lock);
    Bool                 retry    = False;
	ProcBackData         proc     = backendProc;
	int			         addWaits = 0;

	if (heldLocksNum >= MAX_SIMULTANEOUS_LOCKS)
        elog->log(LOG_ERROR, 
		          ERROR_CODE_TOO_MANY_LOCKS_TAKEN, 
				  "too many light locks taken");        

	/* The loop here is needed. From the start we figure out 
	 * if we need to wait or not. If we do not have to wait 
	 * we just acquire the lock. How do we acquire the lock?
	 * First of all we take the lock from the light lock array 
	 * and 
	 */
	CYCLE
	{
		Bool    mustWait;

		/* First of all we need to determine whether we wait or not. 
		 * We need to read and update some properties of the lock.
		 * This is protected by spin lock. Operation of determining whether 
		 * to wait or not is very fast. So applying a spin lock here is 
		 * reasonable. 
		 */
		SPIN_LOCK_ACQUIRE(slm, &lock->mutex);

        if (retry)
			lock->releaseOK = True;

		/* Everyting depends on the lock mode we want to acquire. 
		 * 
		 * If we want to acquire an exclusive lock, we should wait until 
		 * another exclusive lock releases and wait for all shared locks 
		 * to finish their execution. 
		 * (For example we wont to update one row.
         *  Suppose this row is being updated by another backend. So we need to wait.
		 *  Suppose this row is being read by another backend. We also can't update it
		 *  because in this case the reader backend would partly read old memory and 
		 *  partly new. So data would be totally inconsistent.
		 * )
		 * When we want to acquire an exclusive lock 
		 * we do not wait only when there are not exclusive or shared holders.
		 * 
		 * If we want to acquire a shared lock.
		 * When this row is being read by a shared holder we are allowed 
		 * to read too. But when it is being modified by an exclusive backend
		 * we have to wait. So when we want to acquire a shared lock we do not
		 * wait only when there are not exclusive holders.
		 */
		mustWait = True;

		if (mode == LL_Exclusive)
		{	
			if (lock->exclHoldersNum == 0 && lock->sharedHoldersNum == 0)
			{
                lock->exclHoldersNum++;
                mustWait = False;
			}            
		}
		else
		{
            if (lock->exclHoldersNum == 0)
			{
                lock->sharedHoldersNum++;
                mustWait = False;
			}
		}

        if (!mustWait)
			break;

		/* Check is the process data has been set */
		if (proc == NULL)
            elog->log(LOG_PANIC, 
		          ERROR_CODE_BACKEND_PROC_NULL, 
	  			  "Process data structure should be set");    
        
		/* If we are here it means that we have to wait. 
		 * lock structure is taken from the shared memory and it 
		 * contains the list of process backends that are 
		 * waiting for the lock. We add our current process information
		 * to the tail of the queue list.
		 */
        proc->waitingForLightLock = true;
		proc->waitingMode         = mode;
		proc->lightLockNext       = NULL;              

		if (lock->head == NULL)
			lock->head = proc;
		else
			lock->tail->lightLockNext = proc;

        SPIN_LOCK_RELEASE(slm, &lock->mutex);

		/* Wait for the semaphore to get signalled. */
		CYCLE
		{
			semm->lockSemaphore(semm, proc->sem);

			/* lockSemaphore can return because of interrupt reason.
			 * For example it waits for the signal event, and this event
			 * becomes signalled, but not the semaphore. In this case we
             * try to acquire the semaphore lock once more.
			 */
			if (!proc->waitingForLightLock)
				break;

			/* Calculate how many additional waits we need to do. */
			addWaits++;
		}

		retry = True;
	}

	SPIN_LOCK_RELEASE(slm, &lock->mutex);

	heldLightLocks[heldLocksNum++] = type;

	/* If there have been extra waits, we need to fix 
	 * the semaphore's count. Unlock semaphore method 
	 * internally calls release semaphore and it decrements 
	 * the semaphore's usage count.
	 */
	while (addWaits-- > 0)
       semm->unlockSemaphore(semm, proc->sem);
}


 
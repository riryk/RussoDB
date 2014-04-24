
#include "sharedmemmanager.h"

/* Array of all light locks in the shared memory. */
ULightLockPadded* lightLockArray = NULL;
int	              heldLocksNum   = 0;

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
    void*                self,
	ELightLockType       type, 
	ELightLockMode       mode)
{
    ILightLockManager    llm   = (ILightLockManager)self;
	ISharedMemManager    smm   = (ISharedMemManager)llm->sharedMemManager; 
	IErrorLogger         elog  = (IErrorLogger)llm->errorLogger;
    ISpinLockManager     slm   = (ISpinLockManager)llm->spinLockManager;

    volatile LightLock   lock  = &(lightLockArray[type].lock);
    Bool                 retry = False;
	ProcBackData         proc  = backendProc;

	if (heldLocksNum >= MAX_SIMULTANEOUS_LOCKS)
        elog->log(LOG_ERROR, 
		          ERROR_CODE_TOO_MANY_LOCKS_TAKEN, 
				  "too many light locks taken");        

	CYCLE
	{
		Bool    mustWait;

		SPIN_LOCK_ACQUIRE(slm, &lock->mutex);

        if (retry)
			lock->releaseOK = True;

		mustWait = True;

		if (mode == LL_Exclusive)
		{	
			if (lock->exclHoldersNum == 0 && lock->sharedHoldersNum)
			{
                lock->exclHoldersNum++;
                mustWait = False;
			}            
		}
		else
		{
            if (lock->exclHoldersNum == 0)
			{
                lock->exclHoldersNum++;
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
 
        proc->waitingForLightLock = true;
		proc->waitingMode         = mode;
		proc->lightLockNext       = NULL;              
	}

	SPIN_LOCK_RELEASE(slm, &lock->mutex);
}


 
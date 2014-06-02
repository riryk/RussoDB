
#include "semaphorelockmanager.h"
#include "signalmanager.h"
#include "semaphore.h"
#include "errno.h"
#include "errorlogger.h"

#ifdef _WIN32
HANDLE*  semaphoresSet;		/* IDs of semaphore sets acquired so far */
#endif

int  semaphoresNum = 0;     /* Number of semaphore sets acquired so far */
int  semaphoresMax = 100;   /* Max number of semaphores */

const SISemaphoreLockManager sSemaphoreLockManager = 
{ 
	&sErrorLogger,
	&sSignalManager,
	dispatchQueuedSignals,
	semaphoreCreate,
	semaphoresCtor,
	lockSemaphore,
	unlockSemaphore,
	releaseSemaphores,
    tryLockSemaphore
};

const ISemaphoreLockManager semaphoreLockManager = &sSemaphoreLockManager;

void semaphoresCtor(
	void*                 self,
	int                   semasMax, 
	int                   port)
{
    ISemaphoreLockManager _       = (ISemaphoreLockManager)self;
    IErrorLogger          elog    = _->errorLogger;

	semaphoresSet = (HANDLE*)malloc(semasMax * sizeof(HANDLE));
	if (semaphoresSet == NULL)
        elog->log(LOG_FATAL, 
           ERROR_CODE_OUT_OF_MEMORY, 
		   "Out of memory. Last error: %lu",
		   GetLastError());

	semaphoresNum = 0;
	semaphoresMax = semasMax;
}

/* create a new semaphore and assign it to sem parameter. */
void semaphoreCreate(
    void*                 self,
	TSemaphore            sem)
{
	ISemaphoreLockManager _       = (ISemaphoreLockManager)self;
    IErrorLogger          elog    = _->errorLogger;

    HANDLE		          curHandle;
	SECURITY_ATTRIBUTES   secAttrs; 

    if (semaphoresNum >= semaphoresMax)
        elog->log(LOG_FATAL, 
           ERROR_CODE_TOO_MANY_SEMAPHORES, 
		   "too many semaphores created");
    
	memset(&secAttrs, 0, sizeof(secAttrs));
	secAttrs.nLength              = sizeof(secAttrs);
	secAttrs.lpSecurityDescriptor = NULL;
	secAttrs.bInheritHandle       = TRUE;

    /* Create a new semaphore */
	curHandle = CreateSemaphore(&secAttrs, 1, 32767, NULL); 
	if (curHandle != NULL)
	{
        *sem = curHandle;
        semaphoresSet[semaphoresNum++] = curHandle;
		return;
	}

    elog->log(LOG_PANIC, 
       ERROR_CODE_COULD_NOT_CREATE_SEMAPHORE, 
	   "could not create semaphore. error code %lu",
	   GetLastError());
}

#ifdef _WIN32

void releaseSemaphores()
{
	int			i;

	for (i = 0; i < semaphoresNum; i++)
		CloseHandle(semaphoresSet[i]);

	free(semaphoresSet);
}

#endif

#ifdef _WIN32

void lockSemaphore(
    void*          self,
	TSemaphore     sem)
{
	ISemaphoreLockManager _       = (ISemaphoreLockManager)self;
    ISignalManager        signMan = _->signalManager;
	IErrorLogger          elog    = _->errorLogger;

    DWORD		   ret;
	HANDLE		   waitHandles[2];

	waitHandles[0] = signalEvent;
	waitHandles[1] = *sem;

	do
	{
	   /* Wait for the semaphore or the signal event to get signalled */
       ret = WaitForMultipleObjectsEx(2, waitHandles, FALSE, INFINITE, TRUE);

	   /* First of all set error number to error */
       errno = EIDRM;

	   /* If our signal event got signalled, in this case 
	    * we dispatch queued signals.
	    */
	   if (ret == WAIT_OBJECT_0)
	   {
           signMan->dispatchQueuedSignals();
		   errno = EINTR;
	   }

	   /* If result is the second wait handler it means that
	    * we have got the semaphore.
	    */
	   if (ret == WAIT_OBJECT_0 + 1)
	       errno = EINTR;

	} while (errno == EINTR);

	if (errno != EINTR)
        elog->log(LOG_FATAL, 
           ERROR_CODE_CREATE_EVENT_FAILED, 
		   "Could not create signal event: error code %lu",
		   GetLastError());
}

#endif

#ifdef _WIN32

void unlockSemaphore(
    void*          self,
	TSemaphore     sem)
{
    ISemaphoreLockManager _       = (ISemaphoreLockManager)self;
    IErrorLogger          elog    = _->errorLogger;

	if (!ReleaseSemaphore(*sem, 1, NULL))
        elog->log(LOG_FATAL, 
           ERROR_CODE_RELEASE_SEMAPHORE_FAILED, 
		   "Could not release semaphore: error code %lu",
		   GetLastError());        
}

#endif

#ifdef _WIN32

Bool tryLockSemaphore(
    void*          self,
    TSemaphore     sem)
{
	DWORD		   ret;

	ISemaphoreLockManager _    = (ISemaphoreLockManager)self;
    IErrorLogger          elog = _->errorLogger;

	ret = WaitForSingleObject(*sem, SEMAPHORE_WAIT_TIMEOUT);

	if (ret == WAIT_OBJECT_0)
		return False;
	
	if (ret == WAIT_TIMEOUT)
	{
		errno = ERROR_CODE_WAIT_FOR_TIMEOUT;
		return True;
	}

	elog->log(LOG_FATAL, 
		      ERROR_CODE_TRY_SEMAPHORE_LOCK_FAILED,
			  "Attempt to apply semaphore lock failed: error code %lu",
              GetLastError());

	return True;
}

#endif

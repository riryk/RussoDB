
#include "semaphorelockmanager.h"
#include "signalmanager.h"

#ifdef _WIN32
HANDLE*  semaphoresSet;		/* IDs of semaphore sets acquired so far */
#endif

int  semaphoresNum = 0;         /* Number of semaphore sets acquired so far */
int  semaphoresMax;         /* Max number of semaphores */

void semaphoresCtor(int semasMax, int port)
{
	semaphoresSet = (HANDLE*)malloc(maxSemas * sizeof(HANDLE));
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

    if (semaphoresNum >= maxSems)
        elog->log(LOG_FATAL, 
           ERROR_CODE_TOO_MANY_SEMAPHORES, 
		   "too many semaphores created");
    
	memset(secAttrs, 0, sizeof(secAttrs));
	secAttrs.nLength              = sizeof(secAttrs);
	secAttrs.lpSecurityDescriptor = NULL;
	secAttrs.bInheritHandle       = TRUE;

    /* Create a new semaphore */
	curHandle = CreateSemaphore(&secAttrs, 1, 32767, NULL); 
	if (curHandle == NULL)
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

	wh[0] = signalEvent;
	wh[1] = *sem;

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


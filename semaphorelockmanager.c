
#include "semaphorelockmanager.h"
#include "signalmanager.h"

#ifdef _WIN32

void lockSemaphore(TSemaphore sem, Bool interruptOK)
{
    DWORD		 ret;
	HANDLE		 waitHandles[2];

	wh[0] = signalEvent;
	wh[1] = *sem;

	do
	{
	   /* Wait for the semaphore or the signal event to get signalled */
       ret = WaitForMultipleObjectsEx(2, waitHandles, FALSE, INFINITE, TRUE);

	   /* If our signal event got signalled, in this case 
	    * we dispatch queued signals.
	    */
	   if (ret == WAIT_OBJECT_0)

	} while (errno == EINTR);
}

#endif


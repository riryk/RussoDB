
#include "latchmanager.h"
#include "errorlogger.h"
#include "trackmemmanager.h"
#include "signalmanager.h"

const SILatchManager sLatchManager = 
{ 
    &sTrackMemManager,
    &sErrorLogger,
	&sSignalManager,
	initLatch, 
	setLatch,
    resetLatch,
	waitLatch
};

const ILatchManager  latchManager  = &sLatchManager;

#ifdef _WIN32

Latch initLatch(void* self)
{
	ILatchManager   _    = (ILatchManager)self;
	IErrorLogger    elog = _->errorLogger;
	IMemoryManager  mm   = _->memManager;

    SECURITY_ATTRIBUTES   sa;
	Latch                 latch;

	latch = (Latch)mm->alloc(sizeof(SLatch));

	ASSERT_VOID(elog, latch != NULL);

	latch->isset   = False;
	latch->ownerid = 0;
	latch->shared  = True;

    memset(&sa, 0, sizeof(sa)); 
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = True;

	latch->event = CreateEvent(&sa, TRUE, FALSE, NULL);
	if (latch->event == NULL)
		elog->log(LOG_ERROR, 
		          ERROR_CODE_GENERAL, 
				  "CreateEvent failed: error code %lu", 
				  GetLastError());

	return latch;
}

/* Sets a latch into active state. That means 
 * that processes or threads, which have been waiting for  
 * the latch, receive a notification that a latch 
 * has become active.
 */
void setLatch(Latch latch)
{
   	HANDLE		handle;

	/* If the latch is already in active state,
	 * we do not do anything and just return.
	 */
	if (latch->isset)
		return;

	/* Set a latch to True. */
	latch->isset = True;
	handle       = latch->event;

	/* If there is a windows event,
	 * we set it and thus notify all
	 * processes or threads waiting for it.
	 */
	if (handle != NULL)
		SetEvent(handle);
}

void waitLatch(
    void*          self,
	Latch          latch)
{
	ILatchManager  _    = (ILatchManager)self;
	ISignalManager sm   = _->signalManager;
    IErrorLogger   elog = _->errorLogger;

	DWORD		   wait_result;
    HANDLE		   events[2];
	HANDLE		   latchevent = latch->event;
	int			   numevents  = 2;
    int			   result     = 0;

	events[0] = signalEvent;
    events[1] = latchevent;

    sm->dispatchQueuedSignals();

    do
	{
        wait_result = WaitForMultipleObjects(numevents, events, FALSE, INFINITE);
        
        if (wait_result == WAIT_FAILED)
		{
			int lastError = GetLastError();

            elog->log(LOG_ERROR, 
		          ERROR_CODE_WAIT_FOR_MULTIPLE_OBJECTS_FAILED, 
				  "ResetEvent failed: error code %lu", 
	 			  lastError);
		}

        if (wait_result == WAIT_OBJECT_0)
			sm->dispatchQueuedSignals();

		if (wait_result == WAIT_OBJECT_0 + 1)
            break; 
	} 
	while (result == 0);

	return result;
}

void resetLatch(Latch latch)
{
    latch->isset = False;
}

#endif
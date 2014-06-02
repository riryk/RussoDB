
#include "threadhelper.h"
#include "error.h"

sleepFunc  slpFunc          = NULL; 
TEvent     threadStartEvent = NULL;

void threadHelpCtor(
    void*            self,
    sleepFunc        slpFuncParam,
	Bool             includeStartThreadEvent)
{
    IThreadHelper    _    = (IThreadHelper)self;
	IErrorLogger     elog = _->errorLogger;

    slpFunc = slpFuncParam;
    ASSERT_VOID(elog, slpFunc != NULL);

	if (!includeStartThreadEvent)
		return;

	threadStartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (threadStartEvent == NULL)
        elog->log(LOG_FATAL, 
		          ERROR_CODE_CREATE_EVENT_FAILED, 
				  "Could not create signal event: error code %lu",
				  GetLastError());
}

#ifdef _WIN32

void waitForEvent(
	void*          self,
	TEvent         eventToWait)
{ 
	IThreadHelper  _    = (IThreadHelper)self;
	IErrorLogger   elog = _->errorLogger;

	DWORD          result;

    result = WaitForSingleObject(
                eventToWait, 
                INFINITE);   

    switch (result)
	{
	/* In this case we have successfully waited for the event. */
	case WAIT_OBJECT_0:
        elog->log(LOG_LOG, 
		          0, 
				  "Successfully waited for the event"); 
		break;
    
	/* When we are here, probably some error has happened. */
    default:
        elog->log(LOG_ERROR, 
		          ERROR_CODE_WAIT_FOR_SINGLE_OBJECT_FAILED, 
				  "Wait for single object failed: error code %lu",
				  GetLastError()); 
		break;
	}
}

void waitForMultipleEvents(
	void*          self,
	TEvent*        eventsToWait,
	int            eventsCount,
	Bool           waitAll)
{
    IThreadHelper  _    = (IThreadHelper)self;
	IErrorLogger   elog = _->errorLogger;

	DWORD          result;

	result = WaitForMultipleObjects(
		        eventsCount,
                eventsToWait,
                waitAll,
                INFINITE);

    switch (result)
	{
	/* In this case we have successfully waited for the event. */
	case WAIT_OBJECT_0:
        elog->log(LOG_LOG, 
		          0, 
				  "Successfully waited for the events"); 
		break;
    
	/* When we are here, probably some error has happened. */
    default:
        elog->log(LOG_ERROR, 
		          ERROR_CODE_WAIT_FOR_SINGLE_OBJECT_FAILED, 
				  "Wait for single object failed: error code %lu",
				  GetLastError()); 
		break;
	}
}

#endif

TThread startThread(
    void*            self,
	THREAD_FUNC      func, 
	void*            param, 
	TThreadId        threadid)
{
    IThreadHelper _    = (IThreadHelper)self;
	IErrorLogger  elog = _->errorLogger;

    TThread threadHandle = CreateThread(
            NULL, 
            0,    
            func, 
            param,
            0,
            &threadid);
     
    if (threadHandle == NULL)
        elog->log(LOG_ERROR,
			      ERROR_CODE_CREATE_THREAD_FAILED,
			      "could not create a thread: error code %lu",
                  GetLastError());

	return threadHandle;
}

void spinWait(int spinMaxCount)
{
    int  i = 0;

	while (i++ < spinMaxCount)
		;
}




#include "threadhelper.h"

typedef struct SIThreadHelper
{
	IErrorLogger   errorLogger;
} SIThreadHelper, *IThreadHelper;

TThread startThread(
    void*            self,
	ThreadFunc       func, 
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
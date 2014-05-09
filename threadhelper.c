
#include "threadhelper.h"

sleepFunc  slpFunc = NULL; 

void threadHelpCtor(
    void*            self,
    sleepFunc        slpFuncParam)
{
    IThreadHelper    _    = (IThreadHelper)self;
	IErrorLogger     elog = _->errorLogger;

    slpFunc = slpFuncParam;
    ASSERT_VOID(elog, slpFunc != NULL);
}

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





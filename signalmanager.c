
#include "signalmanager.h"

#ifdef _WIN32

HANDLE	          signalEvent;
HANDLE		      signalPipe = INVALID_HANDLE_VALUE;
CRITICAL_SECTION  signalCritSec;

signalFunc        signalArray[SIGNAL_COUNT];
signalFunc        signalDefaults[SIGNAL_COUNT];

volatile int      signalQueue;
int			      signalMask;

DWORD __stdcall signalThread(LPVOID param);

void signalCtor(void* self)
{
	ISignalManager  _    = (ISignalManager)self;
    IErrorLogger    elog = _->errorLogger;

    int			    i;
	HANDLE		    signalThread;

	InitializeCriticalSection(&signalCritSec);

	for (i = 0; i < SIGNAL_COUNT; i++)
	{
		signalArray[i]    = SIGNAL_IGNORE;
		signalDefaults[i] = SIGNAL_IGNORE;
	}
    
	signalQueue = 0;
    signalMask  = 0;
    
	/* Create the global event handle used to flag signals */
	signalEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (signalEvent == NULL)
        elog->log(LOG_FATAL, 
		          ERROR_CODE_CREATE_EVENT_FAILED, 
				  "Could not create signal event: error code %lu",
				  GetLastError());

	/* Create thread for handling signals */
	signalThread = CreateThread(NULL, 0, signalThread, NULL, 0, NULL);
	if (signalThread == NULL)
        elog->log(LOG_FATAL, 
		          ERROR_CODE_CREATE_THREAD_FAILED, 
				  "Could not create signal handler thread: error code %lu",
				  GetLastError());

}

/* Signal handling thread */
DWORD __stdcall signalThread(LPVOID param)
{
    char		pipeName[128];
	HANDLE		pipe = signalPipe;

	snprintf(pipeName, sizeof(pipename), "\\\\.\\pipe\\signal_%lu", GetCurrentProcessId());


}

#endif
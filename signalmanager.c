
#include "signalmanager.h"
#include "error.h"
#include "errorlogger.h"

#ifdef _WIN32

HANDLE	          signalEvent;
HANDLE		      signalPipe = INVALID_HANDLE_VALUE;
CRITICAL_SECTION  signalCritSec;

signalFunc        signalArray[SIGNAL_COUNT];
signalFunc        signalDefaults[SIGNAL_COUNT];

volatile int      signalQueue = 0;
int			      signalMask;

const SISignalManager sSignalManager = 
{
    &sErrorLogger,
	signalCtor,
	dispatchQueuedSignals
};

const ISignalManager  signalManager  = &sSignalManager;

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
		signalArray[i]    = SIGNAL_DEFAULT;
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

	/* Create console control handle to pick up Ctrl-C etc */
	if (!SetConsoleCtrlHandler(consoleHandler, TRUE))
        elog->log(LOG_FATAL, 
		          ERROR_CODE_SET_CONSOLE_CTRL_HANDLER, 
				  "Could not set console control handler: error code %lu",
				  GetLastError());
}

/* Console control handler will execute on a thread 
 * created by the OS at the time of invocation.
 */
BOOL WINAPI consoleHandler(
    void*          self, 
	DWORD          ctrlType)
{
	if (ctrlType == CTRL_C_EVENT ||
		ctrlType == CTRL_BREAK_EVENT ||
		ctrlType == CTRL_CLOSE_EVENT ||
		ctrlType == CTRL_SHUTDOWN_EVENT)
	{
		queueSignal(self, SIGNAL_INTERRUPT);
		return True;
	}

	return False;
}

/* Signal handling thread */
DWORD __stdcall signalThread(LPVOID param)
{
    char		pipeName[128];
	HANDLE		pipe = signalPipe;

	snprintf(pipeName, sizeof(pipeName), "\\\\.\\pipe\\signal_%lu", GetCurrentProcessId());
}

signalFunc signalMain(
    void*          self,
    int            signum, 
	signalFunc     handler)
{
	signalFunc	   prevFunc;

	if (signum >= SIGNAL_COUNT || signum < 0)
		return SIGNAL_ERROR;

	prevFunc            = signalArray[signum];  
	signalArray[signum] = handler;

	return prevFunc;
}

void queueSignal(
    void*          self, 
	int            signum)
{
	if (signum >= SIGNAL_COUNT || signum <= 0)
		return;

	EnterCriticalSection(&signalCritSec);
	signalQueue |= SIGNAL_MASK(signum);
	LeaveCriticalSection(&signalCritSec);

	SetEvent(signalEvent);
}

/* Dispatch all queued signals. */
void dispatchQueuedSignals()
{
    int			i;

	EnterCriticalSection(&signalCritSec);

	while (QUEUE_LEFT)
	{
        /* Get queue mask that represent queued signals. */
		int	   mask = QUEUE_LEFT;   

		for (i = 0; i < SIGNAL_COUNT; i++)
		{
			signalFunc   signl;

			/* If i-th element of the mask is set to true,
			 * we retrieve i-th signal message and process it. 
			 */
            if (!(mask & SIGNAL_MASK(i)))
				continue;
			
            signl = signalArray[i];
            
			/* Set up the default function. */
			if (signl == SIGNAL_DEFAULT)
                signl = signalDefaults[i];

			/* Clear i-th bit in mask bit array. */
            signalQueue &= ~SIGNAL_MASK(i);

			if (signl != SIGNAL_ERROR 
		     && signl != SIGNAL_IGNORE 
			 && signl != SIGNAL_DEFAULT)
			{
                LeaveCriticalSection(&signalCritSec);
				signl(i);
				EnterCriticalSection(&signalCritSec);
				break;
				/* Before executing a signal handler we leave 
				 * the critical section and launch the handler.
				 * Inside the signal handler the signal queue can 
				 * be modified. Also another thread can modify it.
				 * we break the current loop and start the outer loop
				 * again. 
				 */
			}
		}
	}

	/* Set the signal event to unsignalled state. */
	ResetEvent(signalEvent);
    LeaveCriticalSection(&signalCritSec);
}

#endif



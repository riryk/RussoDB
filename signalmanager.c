
#include "signalmanager.h"
#include "error.h"
#include "errorlogger.h"
#include "snprintf.h"
#include "errno.h"

#ifdef _WIN32

HANDLE	          signalEvent;
HANDLE		      signalThread;
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
	dispatchQueuedSignals,
	signalDtor,
    setSignal,
	queueSignal,
	sentSignal
};

const ISignalManager  signalManager  = &sSignalManager;

void signalCtor(void* self)
{
	ISignalManager  _    = (ISignalManager)self;
    IErrorLogger    elog = _->errorLogger;

    int			    i;

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
	signalThread = CreateThread(NULL, 0, signalThreadFunc, NULL, 0, NULL);
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

void signalDtor(void* self)
{
    TerminateThread(signalThread, 0); 
    CloseHandle(signalThread);
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

/* Signal dispatching thread */
DWORD __stdcall signalDispatchThread(LPVOID param)
{
	HANDLE		pipe = (HANDLE) param;
	BYTE		sigNum;
	DWORD		bytes;

	if (!ReadFile(pipe, &sigNum, 1, &bytes, NULL))
	{
		CloseHandle(pipe);
		return 0;
	}

	if (bytes != 1)
	{
		/* Received incorrect message through named pipe. */
		CloseHandle(pipe);
		return 0;
	}

	WriteFile(pipe, &sigNum, 1, &bytes, NULL);	

	FlushFileBuffers(pipe);
	DisconnectNamedPipe(pipe);
	CloseHandle(pipe);

	queueSignal(sigNum);

	return 0;
}

/* Create the signal listener pipe for specified PID */
HANDLE createSignalListener(
	void*          self, 
	int            pid)
{
    ISignalManager _    = (ISignalManager)self;
    IErrorLogger   elog = _->errorLogger;

	char	       pipename[128];
	HANDLE	       pipe;

	snprintf(pipename, 
		     sizeof(pipename), 
			 "\\\\.\\pipe\\signal_%u", 
			 (int)pid);

	pipe = CreateNamedPipe(
		     pipename, 
			 PIPE_ACCESS_DUPLEX,
			 PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			 PIPE_UNLIMITED_INSTANCES, 
			 16, 
			 16, 
			 1000, 
			 NULL);

	if (pipe == INVALID_HANDLE_VALUE)
        elog->log(LOG_ERROR, 
		          ERROR_CODE_CREATE_NAMED_PIPE_FAILED, 
				  "could not create signal listener pipe for process id %d: error code %lu",
                  pid,
				  GetLastError());     

	return pipe;
}

int sentSignal(
	void*          self, 
	int            pid,
	int            signal)
{
    ISignalManager _    = (ISignalManager)self;
    IErrorLogger   elog = _->errorLogger;

    char	       pipename[128];
	BYTE           signalData = signal;
	BYTE           signalRet  = 0;
	DWORD          bytesCount;
    
	if (signal >= SIGNAL_COUNT || signal < 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (pid <= 0)
	{
		errno = EINVAL;
		return -1;
	}
   
    snprintf(pipename, 
		     sizeof(pipename), 
			 "\\\\.\\pipe\\signal_%u", 
			 pid);

    if (CallNamedPipe(pipename, &signalData, 1, &signalRet, 1, &bytesCount, 20000))
	{
		if (bytesCount != 1 || signalRet != signal)
		{
			errno = ESRCH;
			return -1;
		}
		return 0;
	}
    
    if (GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		errno = ESRCH;
		return -1;
	}

	if (GetLastError() == ERROR_ACCESS_DENIED)
	{
		errno = EPERM;
		return -1;
	}
	
	errno = EINVAL;
	return -1;
}

/* Signal handling thread */
DWORD __stdcall signalThreadFunc(LPVOID param)
{
    char		pipeName[128];
	HANDLE		pipe        = signalPipe;
	int         attemptsNum = 0;
	DWORD       procId      = GetCurrentProcessId();

	snprintf(pipeName, 
		     sizeof(pipeName), 
			 "\\\\.\\pipe\\signal_%u", 
			 procId);

	CYCLE
	{
        BOOL		fConnected;
		HANDLE		hThread;
		HANDLE		newpipe;

		if (pipe == INVALID_HANDLE_VALUE)
		{
            pipe = CreateNamedPipe(
				       pipeName, 
				       PIPE_ACCESS_DUPLEX,
					   PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
					   PIPE_UNLIMITED_INSTANCES, 
					   16, 
					   16, 
					   1000, 
					   NULL);    
 
            if (pipe == INVALID_HANDLE_VALUE)
			{
				if (attemptsNum == MAX_CREATE_NAMED_PIPES_ATTEMPTS)
				{
                    fprintf(
						stderr, 
		                "max number of attempts to create a named pipe has been reached",
						MAX_CREATE_NAMED_PIPES_ATTEMPTS);

					return 0;
				}

                attemptsNum++;
                 
			    fprintf(
				    stderr, 
		            "could not create signal listener pipe: error code %lu; retrying\n",
                    GetLastError());

				SleepEx(500, FALSE);
				continue;
			}
		}

		/* Enables a named pipe server process to wait for a client process 
		 * to connect to an instance of a named pipe. 
		 * A client process connects by calling either the CreateFile or CallNamedPipe function. 
		 */
		fConnected = ConnectNamedPipe(pipe, NULL);

        if (!fConnected)
		{
			DWORD  lastErr = GetLastError();

			if (lastErr != ERROR_PIPE_CONNECTED)
			{
                if (attemptsNum == MAX_CREATE_NAMED_PIPES_ATTEMPTS)
			    {
                    fprintf(
					    stderr, 
	                    "max number of attempts to create a named pipe has been reached",
					    MAX_CREATE_NAMED_PIPES_ATTEMPTS);

				    return 0;
			    }

                attemptsNum++;

                fprintf(
			        stderr, 
	                "could not connect to a named pipe: error code %lu; retrying\n",
                    GetLastError());  

				CloseHandle(pipe);
		        pipe = INVALID_HANDLE_VALUE;

				continue;
			}
	    }

		/* If we have a successfully connected named pipe, we 
		 * pass it along to a working thread that will read and process data.
		 * But this process can be very fast and the working thread can 
		 * close the pipe before we create a new one. In this case we can miss
		 * some requests. So we create a new named pipe.
		 */
	    newpipe = CreateNamedPipe(
			          pipeName, 
					  PIPE_ACCESS_DUPLEX,
			          PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
					  PIPE_UNLIMITED_INSTANCES, 
					  16, 
					  16, 
					  1000, 
					  NULL);

		if (newpipe == INVALID_HANDLE_VALUE)
	    { 
            if (attemptsNum == MAX_CREATE_NAMED_PIPES_ATTEMPTS)
		    {
                fprintf(
				    stderr, 
                    "max number of attempts to create a named pipe has been reached",
				    MAX_CREATE_NAMED_PIPES_ATTEMPTS);

			    return 0;
			}

            attemptsNum++;

            fprintf(
		        stderr, 
                "could not create signal listener pipe: error code %lu; retrying\n",
                GetLastError());
		}

		hThread = CreateThread(
			          NULL, 
					  0,
				      (LPTHREAD_START_ROUTINE)signalDispatchThread,
					  (LPVOID)pipe, 
					  0, 
					  NULL);

		if (hThread == INVALID_HANDLE_VALUE)
		{
            if (attemptsNum == MAX_CREATE_NAMED_PIPES_ATTEMPTS)
		    {
                fprintf(
				    stderr, 
                    "max number of attempts to create a named pipe has been reached",
				    MAX_CREATE_NAMED_PIPES_ATTEMPTS);

			    return 0;
			}

            attemptsNum++;  

            fprintf(
		        stderr, 
                "could not create signal dispatch thread: error code %lu;\n",
                GetLastError());
		}
	    else
		    CloseHandle(hThread);

		/* Our background thread is processing our old pipe. 
		 * So we need to substitute pipe with the new one.
		 */
        pipe = newpipe;
	}
}

signalFunc setSignal(
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

void queueSignal(int signum)
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

	while (QUEUE_LEFT())
	{
        /* Get queue mask that represent queued signals. */
		int	   mask = QUEUE_LEFT();   

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



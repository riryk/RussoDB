#include "logger.h"
#include "stdio.h"
#include "osfile.h"
#include "latch.h"

FILE*  logFile       = NULL;

/* stderr has already been redirected for syslogger? */
Bool              redirect_done = False;
Latch             loggerLatch   = NULL;

#ifdef WIN32

HANDLE		      logPipe[2]    = {0, 0};
CRITICAL_SECTION  logSection    = NULL;
HANDLE            threadHandle  = NULL;

#endif

/* Writes a message directly to an opened log file */
void write_message_file(
	char*               buffer, 
	int                 count)
{
	int			result;

	result = fwrite(buffer, 1, count, logFile);

	/* We have not written the message. 
	 * We should report an error.
	 */
	if (result != count)
		; // Should report an error
}

void logger_main(void*  self)
{
	ILogger        _  = (ILogger)self;
    ILatchManager  lm = (ILatchManager)_->latchManager;
	IErrorLogger   el = (IErrorLogger)_->errorLogger;

    if (redirect_done)
	{
		int fd = openFileBase("nul", O_WRONLY, 0);

		close(fileno(stdout));
		close(fileno(stderr));

		if (fd != -1)
		{
			dup2(fd, fileno(stdout));
			dup2(fd, fileno(stderr));
			close(fd);
		}
	}
#ifdef WIN32

	else
		_setmode(_fileno(stderr), _O_TEXT);
 
    if (syslogPipe[1] != NULL)
        CloseHandle(syslogPipe[1]);

    syslogPipe[1] = 0;

#endif

    lm->initLatch(lm, loggerLatch);

#ifdef WIN32
    
    InitializeCriticalSection(&logSection);
    EnterCriticalSection(&logSection);
    
    threadHandle = (HANDLE)_beginthreadex(NULL, 0, pipeThread, NULL, 0, NULL);
	if (threadHandle == 0)
        elog->log(LOG_FATAL, 
		          ERROR_CODE_CREATE_THREAD_FAILED, 
				  "could not create syslogger data transfer thread");

#endif

}

/* This method transfers data from the pipe to the current log file. */
uint __stdcall pipeThread(void *arg)
{
	char   logbuffer[READ_BUF_SIZE];
	int	   bytes_in_logbuffer = 0;
    


    return 0;    
}


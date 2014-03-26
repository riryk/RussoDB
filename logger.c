#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "logger.h"
#include "stdio.h"
#include "osfile.h"
#include "latch.h"
#include "nodes.h"
#include "ilogger.h"
#include "snprintf.h"

FILE*  logFile       = NULL;

/* stderr has already been redirected for syslogger? */
Bool              redirect_done       = False;
Latch             loggerLatch         = NULL;
char*             loggerDirectory     = NULL;
int64             loggerFileTimeFirst = 0;

#ifdef WIN32

/*
 * We really want line-buffered mode for logfile output, but Windows does
 * not have it, and interprets _IOLBF as _IOFBF (bozos).  
 * So use _IONBF instead on Windows.
 */
#define LOG_BUF_MODE _IONBF

HANDLE		      logPipe[2]    = {0, 0};
CRITICAL_SECTION  logSection;
HANDLE            threadHandle  = NULL;
List*             buffer_lists[BUFFER_LISTS_COUNT];

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
	ILogger        _    = (ILogger)self;
    ILatchManager  lm   = (ILatchManager)_->latchManager;
	IErrorLogger   elog = (IErrorLogger)_->errorLogger;

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
 
    if (logPipe[1] != NULL)
        CloseHandle(logPipe[1]);

    logPipe[1] = 0;

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
    
    CYCLE
	{
		lm->resetLatch(loggerLatch);
        break;
	}
}

char* getLogFileName(
    void*           self,
	int             time)
{
    ILogger         _      = (ILogger)self;
	IMemoryManager  memman = _->memManager;

    char*           filename;

    filename = (char*)memman->alloc(MAX_PATH);
    snprintf(filename, MAX_PATH, "%s/%u", loggerDirectory, time);

	return filename;
}

FILE* logFileOpen(
    void*           self,
	char*           filename,
	char*           mode)
{
	ILogger        _    = (ILogger)self;
	IErrorLogger   elog = (IErrorLogger)_->errorLogger;

    FILE*       fhd;
	int         savedErr;

	fhd    = fopen(filename, mode);

	if (fhd != NULL)
	{
		setvbuf(fhd, NULL, LOG_BUF_MODE, 0);

#ifdef WIN32
		_setmode(_fileno(fhd), _O_TEXT);
#endif
		return fhd;
	}

	savedErr = errno;

	elog->log(LOG_ERROR, 
		      ERROR_CODE_FILE_ACCESS, 
			  "could not open log file \"%s\"", 
			  filename);

	errno = savedErr;
}

void logger_start(void*  self)
{
	ILogger          _      = (ILogger)self;
	IErrorLogger     elog   = (IErrorLogger)_->errorLogger;
	IMemoryManager   mm     = (IMemoryManager)_->memManager;
	IProcessManager  prcman = (IProcessManager)_->processManager;

	char*  filename;
	int    res;
	int	   fileDesc;

    /* First time we enter here we create the pipe that will
	 * receive all information from all stderrs from all processes.
	 */
    if (logPipe[0] != NULL)
	{
        SECURITY_ATTRIBUTES sa;

		memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;

        if (!CreatePipe(&(logPipe[0]), &(logPipe[1]), &sa, 32768))
            elog->log(LOG_ERROR, 
		          ERROR_CODE_CREATE_PIPE_FAILED, 
				  "could not create a logger pipe");
	}

	mkdir(loggerDirectory);
	loggerFileTimeFirst = time(NULL);
	filename = getLogFileName(_, loggerFileTimeFirst);

	logFile  = logFileOpen(_, filename, "a");
	mm->free(filename);

    res = prcman->startSubProcess(prcman, 0, NULL);
	if (res == -1)
        elog->log(LOG_ERROR, 
		      ERROR_CODE_START_SUB_PROC_FAILED, 
			  "could not start a subprocess");

    if (redirect_done)
		return;

	/* Do stderr redirection. */
    fflush(stderr);
	fileDesc = _open_osfhandle((intptr_t)syslogPipe[1], 
		                       _O_APPEND | _O_BINARY);

    if (dup2(fileDesc, _fileno(stderr)) < 0)   
        elog->log(LOG_ERROR, 
		      ERROR_CODE_STDERR_REDIRECT_FAILED, 
			  "could not redirect stderr");

	close(fileDesc);
    _setmode(_fileno(stderr), _O_BINARY);
  
    syslogPipe[1] = 0;
#endif
				redirection_done = true;
}

void processLogBuffer(
	void*       self,
    char*       buf, 
	int*        buf_bytes)
{
	ILogger         _       = (ILogger)self;
	IStringManager  strman  = _->stringManager;
	IMemoryManager  memman  = _->memManager;
	IListManager    listman = _->listManager;

    char*       cursor = buf;
	int			count  = *buf_bytes;
	int			dest   = LOG_DESTINATION_STDERR;   

    /* While we have enough for a header, process data... */
	while (count >= (int)sizeof(SPipeChunkHeader))
	{
        SPipeChunkHeader  hdr;
		int			      chunklen;
		Bool              isHeaderValid;
		Bool              isChunkLast;

        List              buf_list;
		ListCell          cell;
        Buffer            existing_buf = NULL;
        Buffer            free_buf  = NULL;
        StringInfo        str;

        /* Do we have a valid header? */
		memcpy(&hdr, cursor, sizeof(SPipeChunkHeader));

		isHeaderValid = hdr.nuls[0] == '\0' 
			         && hdr.nuls[1] == '\0'
					 && hdr.len > 0
					 && hdr.len <= PIPE_CHUNK_MAX_LOAD
					 && hdr.pid != 0 &&
					 (hdr.isLast == 't' || hdr.isLast == 'f' ||
					  hdr.isLast == 'T' || hdr.isLast == 'F');
        
		/* Process a non-protocol message */
        if (!isHeaderValid)
		{
			for (chunklen = 1; chunklen < count; chunklen++)
			{
				if (cursor[chunklen] == '\0')
					break;
			}

			/* fall back on the stderr log as the destination */
			write_message_file(cursor, hdr.len);

			cursor += chunklen;
			count  -= chunklen;
		}

        /* Process a protocol message */    
		chunklen = PIPE_CHUNK_HEADER_SIZE + hdr.len;
            
		if (count < chunklen)
			break;

		buf_list = buffer_lists[hdr.pid % BUFFER_LISTS_COUNT];

        foreach(cell, buf_list)
		{
            Buffer buf = (Buffer)list_first(cell);

			if (buf->proc_id == hdr.pid)
			{
				existing_buf = buf;
				break;
			}

			if (buf->proc_id == 0 && free_buf == NULL)
                free_buf = buf;
		}

        isChunkLast = hdr.isLast == 'f' || hdr.isLast == 'F';

		/* If the buffer is not the last */
		if (isChunkLast)
		{
			/* Save a complete non-final chunk in a per process id buffer */
            if (existing_buf != NULL)
			{
                /* Add the chunk to the preceding chunk */ 
				str = &(existing_buf->data);

				strman->appendStringInfoBinary(
					      strman, 
						  str, 
						  cursor + PIPE_CHUNK_HEADER_SIZE,
						  hdr.len);
				break;
			}

            /* We have not found an existing buffer where to put a buffer.
			 * Try to put it into a free buffer.
			 */
            if (free_buf == NULL)
			{
				free_buf     = (Buffer)memman->alloc(sizeof(SBuffer));
                buf_list     = listman->listAppend(listman, buf_list, free_buf);
				buffer_lists[hdr.pid % BUFFER_LISTS_COUNT] = buf_list;
			}

			free_buf->proc_id = hdr.pid;
			str               = &(free_buf->data);

			strman->initStringInfo(strman, str);
            
            strman->appendStringInfoBinary(
					      strman, 
						  str, 
						  cursor + PIPE_CHUNK_HEADER_SIZE,
						  hdr.len);

			cursor += chunklen;
		    count  -= chunklen;

			break;
	    }

        /* Process the final chunk */
        if (existing_buf == NULL)
		{
			/* The whole message was one chunk, evidently. */
		    write_message_file(
				cursor + PIPE_CHUNK_HEADER_SIZE, 
				hdr.len);

			break;
		}

		str = &(existing_buf->data);

        strman->appendStringInfoBinary(
				      strman, 
					  str, 
					  cursor + PIPE_CHUNK_HEADER_SIZE,
					  hdr.len);

		write_message_file(str->data, str->len);
        
		existing_buf->proc_id = 0;
		memman->free(str->data);

		cursor += chunklen;
		count  -= chunklen;

		break;
	}

    /* We don't have a full chunk, so left-align what remains in the buffer */
	if (count > 0 && cursor != buf)
		memmove(buf, cursor, count);

	*buf_bytes = count;
}

/* This method transfers data from the pipe to the current log file. */
uint __stdcall pipeThread(
    void*       self,
	void*       arg)
{
	char   buf[READ_BUF_SIZE];
	int	   bufbytes = 0;
    
    ILogger      _    = (ILogger)self;
    IErrorLogger elog = (IErrorLogger)_->errorLogger;

    CYCLE
	{
        DWORD	bytesRead;
		BOOL    result;

        result = ReadFile(logPipe[0],
						  buf + bufbytes,
						  sizeof(buf) - bufbytes,
						  &bytesRead, 
						  0); 

        EnterCriticalSection(&logSection);

        if (!result)
		{
            DWORD		error = GetLastError();

			if (error == ERROR_HANDLE_EOF ||
				error == ERROR_BROKEN_PIPE)
				break;
            
            elog->log(LOG_LOG, 
		          ERROR_CODE_FILE_ACCESS, 
				  "could not read from logger pipe");
		}

		if (bytesRead > 0)
		{
            bufbytes += bytesRead;

		}

		LeaveCriticalSection(&logSection);
	}

    return 0;    
}


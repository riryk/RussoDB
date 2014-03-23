#include "processhelper.h"

HANDLE		  sharedMemID       = 0;
void*         sharedMemAddr     = NULL;
size_t        sharedMemSegmSize = 0;

#ifdef _WIN32

/* Duplicate a handle for usage in a child process, 
 * and write the child process instance 
 * of the handle to the parameter file.
 */
Bool getDuplicatedHandle(
	void*            self,
	HANDLE*          dest, 
	HANDLE           src, 
	HANDLE           childProcess)
{
    IProcessManager _      = (IProcessManager)self;
	IErrorLogger    elog   = _->errorLogger;
	HANDLE		    hChild = INVALID_HANDLE_VALUE;

	if (!DuplicateHandle(GetCurrentProcess(),
						 src,
						 childProcess,
						 &hChild,
						 0,
						 TRUE,
						 DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
	{
		elog->log(LOG_LOG,
			      ERROR_CODE_DUPLICATE_HANDLE_FAILED,
			      "could not duplicate handle to be written to backend parameter file: error code %lu",
                  GetLastError());

		return False;
	}

	*dest = hChild;
	return True;
}

#endif

#ifdef _WIN32

/* Create a pipe for listening signals */
HANDLE createSignalListener(void* self, int pid)
{
    IProcessManager _    = (IProcessManager)self;
	IErrorLogger    elog = _->errorLogger;

	char		pipename[128];
	HANDLE		pipe;

	snprintf(pipename, sizeof(pipename), "\\\\.\\pipe\\signal_%u", (int)pid);

	pipe = CreateNamedPipe(pipename, 
		                   PIPE_ACCESS_DUPLEX,
					       PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
						   PIPE_UNLIMITED_INSTANCES, 
						   16, 
						   16, 
						   1000, 
						   NULL);

	if (pipe == INVALID_HANDLE_VALUE)
        elog->log(LOG_ERROR, 
		          ERROR_CODE_CREATE_PIPE_FAILED, 
				  "could not create signal listener pipe for PID %d: error code %lu", 
                  pid,
				  GetLastError());

	return pipe;
}

#endif

#ifdef _WIN32

Bool reserveSharedMemoryRegion(
    void*            self,
	HANDLE           childProcess)
{
    IProcessManager  _    = (IProcessManager)self;
	IErrorLogger     elog = _->errorLogger;

    void*            address;

	ASSERT(elog, sharedMemAddr != NULL, -1);
    ASSERT(elog, sharedMemSegmSize != 0, -1);

	address = VirtualAllocEx(
		         childProcess, 
				 sharedMemAddr, 
				 sharedMemSegmSize,
				 MEM_RESERVE, 
				 PAGE_READWRITE);

	if (address == NULL)
	{   
        elog->log(LOG_ERROR, 
		          ERROR_CODE_RESERVE_MEMORY_FAILED, 
				  "could not reserve shared memory region (addr=%p) for child %p: error code %lu", 
                  sharedMemAddr, 
                  childProcess,
				  GetLastError());

		return False;
	}

    if (address != sharedMemAddr, )
	{
		/*
		 * Should never happen - in theory if allocation granularity causes
		 * strange effects it could, so check just in case.
		 *
		 * Don't use FATAL since we're running in the postmaster.
		 */
		elog(LOG, "reserved shared memory region got incorrect address %p, expected %p",
			 address, UsedShmemSegAddr);
		VirtualFreeEx(hChild, address, 0, MEM_RELEASE);
		return false;
	}

	return true;
}

#endif

Bool fillBackandParams(
    void*            self,
	BackendParams    param, 
	HANDLE           childProcess, 
	int              childPid)
{
	HANDLE signListener;

	strcpy(param->dataDir, DataDir, MAX_PATH);
	memcpy(param->listenSockets, &ListenSockets, sizeof(socket_type));

	param->cancelKey = CancelKey;;
	param->childSlot = ChildSlot;

	param->segmId    = NULL;
	param->segmAddr  = NULL;
	param->processId = ProcId;

	param->startTime      = StartTime;
	param->reloadTime     = ReloadTime;
    param->loggerFileTime = LoggerFileTime;

	param->redirectDone           = RedirectDone;
	param->IsBinaryUpgrade        = False;
	param->maxSafeFileDescriptors = maxFileDescriptors;
	param->masterHandle           = NULL;

#ifdef _WIN32

	signListener = createSignalListener(self, childPid);

    if (!getDuplicatedHandle(
		    self,
			param->initPipe, 
	        signListener, 
	        childProcess))
		return False;

#endif

	memcpy(&param->logPipe, logPipe, sizeof(logPipe));
    strlcpy(param->execPath, ExecPath, MAX_PATH);
	return True;
}

int startSubProcess(void* self, int argc, char* argv[])
{
	IProcessManager _    = (IProcessManager)self;
	IErrorLogger    elog = _->errorLogger;

    STARTUPINFO          si;
	PROCESS_INFORMATION  pi;
	SECURITY_ATTRIBUTES  sa;
	BackendParams		 paramSm;
	char		         paramSmStr[32];
	char                 commandLine[MAX_PATH * 2];
	int                  cmdCharCount;
	HANDLE               paramMap;
	int                  i, j;

    ASSERT(elog, argv[2] == NULL, -1); 

    /* Set up shared memory for parameter passing */
	ZeroMemory(&sa, sizeof(sa));
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = TRUE;
    
	/* If the first parameter is INVALID_HANDLE_VALUE, 
	 * the calling process must also specify a size 
	 * for the file mapping object. In this scenario, 
	 * CreateFileMapping creates a file mapping object 
	 * of a specified size that is backed by the system paging file 
	 * instead of by a file in the file system.
	 */
	paramMap = CreateFileMapping(INVALID_HANDLE_VALUE,
			 					 &sa,
								 PAGE_READWRITE,
								 0,
								 sizeof(SBackendParams),
								 NULL);

    if (paramMap == INVALID_HANDLE_VALUE)
	{
        elog->log(LOG_ERROR, 
		          ERROR_CODE_CREATE_FILE_MAP_FAILED, 
				  "could not create file mapping: error code %lu", 
				  GetLastError());

		return -1;
	}
    
	/* Maps a view of a file mapping into the address space of a calling process. */
	paramSm = MapViewOfFile(paramMap, FILE_MAP_WRITE, 0, 0, sizeof(SBackendParams));
	if (!paramSm)
	{
        elog->log(LOG_ERROR, 
		          ERROR_CODE_MAP_MEMORY_TO_FILE, 
				  "could not map backend parameter memory: error code %lu", 
				  GetLastError());

		CloseHandle(paramSm);
		return -1;
	}

	sprintf(paramSmStr, "%lu", (DWORD)paramSm);
    argv[2] = paramSmStr;
    
	cmdCharCount = sizeof(commandLine);
	commandLine[cmdCharCount - 1] = '\0';
    commandLine[cmdCharCount - 2] = '\0';

	snprintf(commandLine, cmdCharCount - 1, "\"%s\"", ExecPath);

	i = 0;
	while (argv[++i] != NULL)
	{
        j = strlen(commandLine);
        snprintf(commandLine + j, sizeof(commandLine) - 1 - j, " \"%s\"", argv[i]);
	}

    if (commandLine[sizeof(commandLine) - 2] != '\0')
	{
        elog->log(LOG_ERROR, 
		          ERROR_CODE_PROC_CMD_LINE_TO_LONG, 
				  "subprocess command line too long");
		return -1;
	}

    memset(&pi, 0, sizeof(pi));
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	/* Create the subprocess in a suspended state. 
	 * This will be resumed later,
	 * once we have written out the parameter file.
	 */
	if (!CreateProcess(NULL, commandLine, NULL, 
		               NULL, TRUE, CREATE_SUSPENDED,
					   NULL, NULL, &si, &pi))
	{
        elog->log(LOG_ERROR, 
		          ERROR_CODE_CREATE_PROCESS_FAILED, 
				  "CreateProcess call failed: (error code %lu)",
				  GetLastError());

		return -1;
	}

	if (!fillBackandParams(
		    _,
	        paramSm, 
	        pi.hProcess, 
			pi.dwProcessId))
	{
        /* Delete the process */
		if (!TerminateProcess(pi.hProcess, 255))
            elog->log(LOG_ERROR, 
		              ERROR_CODE_TERMINATE_PROCESS_FAILED, 
				      "Terminate process failed: (error code %lu)",
				      GetLastError());

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return -1;			
	}

    /* Drop the handler to the shared memory.
	 * Now the shared memory has already been passed
	 * along to the child process.
	 */
	if (!UnmapViewOfFile(paramSm))
        elog->log(LOG_ERROR, 
		          ERROR_CODE_UNMAP_VIEW_OF_FILE, 
				  "could not unmap view of backend parameter file: error code %lu",
				  GetLastError());

	if (!CloseHandle(paramMap))
        elog->log(LOG_ERROR, 
		          ERROR_CODE_CLOSE_HANDLER_FAILED, 
				  "could not close handle to backend parameter file: error code %lu",
				  GetLastError());

    /*
	 * Reserve the memory region used by our main shared memory segment before
	 * we resume the child process.
	 */
	if (!pgwin32_ReserveSharedMemoryRegion(pi.hProcess))
	{

	}
} 

#endif
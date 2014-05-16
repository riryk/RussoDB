#include "processhelper.h"
#include "errorlogger.h"

#ifdef _WIN32
#include "TlHelp32.h"
#endif

#include <stdio.h>
#include <stdlib.h>

HANDLE		  sharedMemID           = 0;
void*         sharedMemAddr         = NULL;
size_t        sharedMemSegmSize     = 0;
TCHAR         sharedMemParamsName[] = TEXT("Global\\SharedMemParams");

ProcBackData  backendProc = NULL;

const SIProcessManager sProcessManager = 
{ 
	&sErrorLogger,
	startSubProcess,
    subProcessMain,
    killAllSubProcesses
};

const IProcessManager processManager = &sProcessManager;

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
			&(param->initPipe), 
	        signListener, 
	        childProcess))
		return False;

#endif

	memcpy(&param->logPipe, logPipe, sizeof(logPipe));
    strcpy(param->execPath, ExecPath, MAX_PATH);
	return True;
}

#ifdef _WIN32

/* This code gets executed when a child process is terminated. */
void WINAPI deadChildProcCallBack(
	PVOID          lpParameter, 
	BOOLEAN        timeoutHappened)
{
    DeadChildInfo  childInfo = (DeadChildInfo)lpParameter;
    DWORD          exitcode;

	if (timeoutHappened)  /* Timeout happened. */
		return;

	/* Unregister the wait handler. */
	UnregisterWaitEx(childInfo->waitHandle, NULL);
    
	/* Retrieves the termination status of the specified process. */
    if (!GetExitCodeProcess(childInfo->procHandle, &exitcode))
	{
		fprintf(stderr, "could not read exit code for process\n");
        exitcode = 255;
	}
}

#ifdef _WIN32

/* Set a privilege to a token. */
BOOL SetPrivilege(
    void*            self,
    HANDLE           hToken,             /* access token handle */
    LPCTSTR          lpszPrivilege,      /* name of privilege to enable/disable */
    BOOL             bEnablePrivilege)   /* to enable or disable privilege */
{
	IProcessManager  _    = (IProcessManager)self;
	IErrorLogger     elog = _->errorLogger;

    TOKEN_PRIVILEGES tp, prevstate;
    LUID             luid;
	DWORD            retlen;

	/* lookup privilege on local system */
    if (!LookupPrivilegeValue( 
            NULL,            
            lpszPrivilege,   
            &luid))       
    {
        elog->log(LOG_ERROR, 
		          ERROR_CODE_LOOKUP_PRIVILEGE_FAILED, 
				  "lookup privilege value error: error code %lu", 
				  GetLastError());

        return FALSE; 
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;

    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

	/* Enable or disable the privilege. */
    if (!AdjustTokenPrivileges(
            hToken, 
            FALSE, 
            &tp, 
            sizeof(TOKEN_PRIVILEGES), 
            &prevstate, 
            &retlen))
    { 
		int error = GetLastError();

        elog->log(LOG_ERROR, 
		          ERROR_CODE_ADJUST_PRIVILEGE_FAILED, 
				  "adjust privilege value error: error code %lu", 
				  error);

        return FALSE; 
    } 

    return TRUE;
}

#endif

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
	DeadChildInfo        childInfo;
    HANDLE               tokenHandle = NULL;

	int                  i, j;

	ASSERT(elog, argv != NULL, -1); 
    ASSERT(elog, argv[0] != NULL, -1); 
    ASSERT(elog, argv[1] != NULL, -1); 
    ASSERT(elog, argv[2] == NULL, -1); 

    /* Set up shared memory for parameter passing */
	ZeroMemory(&sa, sizeof(sa));
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = TRUE;

    if (!OpenProcessToken(
		   GetCurrentProcess(), 
		   TOKEN_ALL_ACCESS, 
		   &tokenHandle))
	{
        elog->log(LOG_ERROR, 
		          ERROR_CODE_OPEN_PROCESS_TOKEN, 
				  "could not open process token: error code %lu", 
				  GetLastError());

		return -1;
	}

    if (!SetPrivilege(
           _,
           tokenHandle, 
           SE_CREATE_GLOBAL_NAME,
           True))
	{
        elog->log(LOG_ERROR, 
		          ERROR_CODE_SET_PRIVILEDGE_FAILED,
				  "could not open process token: error code %lu", 
				  GetLastError());

		return -1;
	}

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
								 sharedMemParamsName);

    if (paramMap == NULL || paramMap == INVALID_HANDLE_VALUE)
	{
		int error = GetLastError();

        elog->log(LOG_ERROR, 
		          ERROR_CODE_CREATE_FILE_MAP_FAILED, 
				  "could not create file mapping: error code %lu", 
				  error);

		return -1;
	}
    
	sharedMemID       = paramMap;
    sharedMemSegmSize = sizeof(SBackendParams);

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
     * If this parameter TRUE, each inheritable handle 
	 * in the calling process is inherited by the new process. 
	 * If the parameter is FALSE, the handles are not inherited. 
	 * Note that inherited handles have the same value 
	 * and access rights as the original handles.
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

    /* All variables are written out, so we can resume the thread */
	if (ResumeThread(pi.hThread) == -1)
	{
        /* Delete the process */
		if (!TerminateProcess(pi.hProcess, 255))
		{
            elog->log(LOG_ERROR, 
		              ERROR_CODE_TERMINATE_PROCESS_FAILED, 
				      "Terminate process failed: (error code %lu)",
				      GetLastError()); 

            CloseHandle(pi.hProcess);
		    CloseHandle(pi.hThread);
		    return -1;			
		}

        CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

        elog->log(LOG_ERROR, 
		          ERROR_CODE_RESUME_THREAD_FAILED, 
				  "Terminate process failed: (error code %lu)",
				  GetLastError());  
		return -1;
	}

	childInfo = malloc(sizeof(SDeadChildInfo));
	if (childInfo == NULL)
        elog->log(LOG_FATAL, 
		          ERROR_CODE_OUT_OF_MEMORY, 
				  "out of memory");   

    childInfo->procHandle = pi.hProcess;
	childInfo->procId     = pi.dwProcessId;

	/* Directs a wait thread in the thread pool to wait on the object. 
	 * The wait thread queues the specified callback function to the thread pool 
	 * when one of the following occurs:
	 *  - The specified object is in the signaled state.
	 *  - The time-out interval elapses.
	 * When a process terminates, the state of the process object 
	 * becomes signaled, releasing any threads 
	 * that had been waiting for the process to terminate.
	 */
    if (!RegisterWaitForSingleObject(
		    &childInfo->waitHandle,
			pi.hProcess,
			deadChildProcCallBack,
			childInfo,
			INFINITE,
		    WT_EXECUTEONLYONCE | WT_EXECUTEINWAITTHREAD))
         elog->log(LOG_ERROR, 
		         ERROR_CODE_REGISTER_WAIT_HANDLER_FAILED, 
				 "Could not register process for wait: error code %lu",
				 GetLastError());  
    
	/* Don't close pi.hProcess here - 
	 * the wait thread needs access to it. 
	 */
	CloseHandle(pi.hThread);

	return pi.dwProcessId;
} 

#endif

#ifdef _WIN32

int subProcessMain(void* self, int argc, char* argv[])
{
	IProcessManager  _    = (IProcessManager)self;
	IErrorLogger     elog = _->errorLogger;

    char             str[80];
    BackendParams    paramSm;
    HANDLE           hMapFile;
   
    printf("Enter a string: ");
    fgets(str, 10, stdin);

	hMapFile = OpenFileMapping(
		           FILE_MAP_ALL_ACCESS, 
				   FALSE,
                   sharedMemParamsName);

    if (hMapFile == NULL)
        elog->log(LOG_ERROR, 
		          ERROR_CODE_FILE_OPEN_MAPPING_FAILED, 
				  "could not open file mapping object: error code %lu",
				  GetLastError());

	/* Maps a view of a file mapping into the address space of a calling process. */
	paramSm = (BackendParams)
		          MapViewOfFile(
				     hMapFile, 
					 FILE_MAP_ALL_ACCESS,
					 0, 
					 0, 
					 sizeof(SBackendParams));
	if (!paramSm)
	{
        elog->log(LOG_ERROR, 
		          ERROR_CODE_MAP_MEMORY_TO_FILE, 
				  "could not map backend parameter memory: error code %lu", 
				  GetLastError());

		CloseHandle(paramSm);
		return -1;
	}

	if (!UnmapViewOfFile(paramSm))
        elog->log(LOG_ERROR, 
		          ERROR_CODE_UNMAP_VIEW_OF_FILE, 
				  "could not unmap view of backend parameter file: error code %lu",
				  GetLastError());

	if (!CloseHandle(hMapFile))
        elog->log(LOG_ERROR, 
		          ERROR_CODE_CLOSE_HANDLER_FAILED, 
				  "could not close handle to backend parameter file: error code %lu",
				  GetLastError());
}

#endif

#ifdef _WIN32

void killAllSubProcesses()
{
    PROCESSENTRY32     pe;
    HANDLE             hSnap;
	BOOL               bContinue = TRUE;

    DWORD currProcId = GetCurrentProcessId();    

    memset(&pe, 0, sizeof(PROCESSENTRY32));
    pe.dwSize = sizeof(PROCESSENTRY32);

	/* Takes a snapshot of the specified processes, 
	 * as well as the heaps, modules, and threads used by these processes. 
	 * If the first parameter is TH32CS_SNAPPROCESS then
	 * all processes in the system in the snapshot. 
	 */
    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (!Process32First(hSnap, &pe))
		return;

    while (bContinue) 
	{
		HANDLE hChildProc;

		/* Select only processes that are descendants 
		 * of the current process.
		 */
		if (pe.th32ParentProcessID == currProcId)
		{
		    hChildProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
		    if (hChildProc != NULL)
		    {
			   TerminateProcess(hChildProc, 1);
			   CloseHandle(hChildProc);
		    }
		}

        bContinue = Process32Next(hSnap, &pe); 
	}
}

#endif
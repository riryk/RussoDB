#include "processhelper.h"

#ifdef _WIN32

int startSubProcess(void* self, int argc, char* argv[])
{
	IProcessManager _    = (IProcessManager)self;
	IErrorLogger    elog = _->errorLogger;

    STARTUPINFO          si;
	PROCESS_INFORMATION  pi;
	SECURITY_ATTRIBUTES  sa;
	HANDLE		         paramSm;
	char		         paramSmStr[32];
	char                 commandLine[MAX_PATH * 2];
	int                  cmdCharCount;
	LVOID                paramMap;
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
	paramSm = CreateFileMapping(INVALID_HANDLE_VALUE,
								&sa,
								PAGE_READWRITE,
								0,
								sizeof(SBackendParams),
								NULL);

    if (paramSm == INVALID_HANDLE_VALUE)
	{
        elog->log(LOG_LOG, 
		          ERROR_CODE_CREATE_FILE_MAP_FAILED, 
				  "could not create file mapping: error code %lu", 
				  GetLastError());

		return -1;
	}
    
	/* Maps a view of a file mapping into the address space of a calling process. */
	paramMap = MapViewOfFile(paramSm, FILE_MAP_WRITE, 0, 0, sizeof(SBackendParams));
	if (!param)
	{
        elog->log(LOG_LOG, 
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


} 

#endif
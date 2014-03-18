#include "processhelper.h"

#ifdef _WIN32

int startSubProcess(void* self, int argc, char* argv[])
{
    STARTUPINFO          si;
	PROCESS_INFORMATION  pi;
	SECURITY_ATTRIBUTES  sa;
	HANDLE		         paramSm;

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
		elog(LOG, "could not create backend parameter file mapping: error code %lu",
			 GetLastError());
		return -1;
	}
}

#endif
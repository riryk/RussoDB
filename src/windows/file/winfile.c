#include "osfile.h"

/* Here we map c library flags needed to open file 
 * to the appropriate flags specific for Windows */
static int MapCreationDisposition(int OpenFlags)
{
	int OpenFlagsWin = OpenFlags & (O_CREAT | O_TRUNC | O_EXCL);

	/* File open flags are not included */
	if (!OpenFlagsWin)
		return 0;

	if (OpenFlagsWin == O_EXCL)                      return OPEN_EXISTING;
	if (OpenFlagsWin == O_CREAT)                     return OPEN_ALWAYS;
	if (OpenFlagsWin == O_TRUNC | O_EXCL)            return TRUNCATE_EXISTING;
    if (OpenFlagsWin == O_CREAT | O_TRUNC)           return CREATE_ALWAYS;
    if (OpenFlagsWin == O_CREAT | O_TRUNC | O_EXCL)  return CREATE_NEW;
	
	return 0;
}

int openFileBase(char *FileName, int FileFlags,...)
{
	int			        FileDescriptor;
	HANDLE		        FileHandler = INVALID_HANDLE_VALUE;
	SECURITY_ATTRIBUTES sa;
	int			        i = 0;

	DWORD               NeededFlags = 
                          O_RDONLY      /* open for reading only */
						| O_WRONLY      /* open for writing only */
                        | O_RDWR        /* open for reading and writing */
						| O_APPEND      /* writes done at eof */ 
                        | O_RANDOM      /* file access is primarily random */
						| O_SEQUENTIAL  /* file access is primarily sequential */
						| O_TEMPORARY   /* temporary file bit */
                        |_O_SHORT_LIVED /* temporary storage file, try not to flush */
                        |_O_NOINHERIT   /* child process doesn't inherit file */
						| O_FILE_DIRECT   
                        |_O_CREAT       /* create and open file */
                        |_O_TRUNC       /* open and truncate */ 
						|_O_EXCL        /* open only if file doesn't already exist */
                        | O_BINARY      /* file mode is binary (untranslated) */
						| O_TEXT;      /* file mode is UTF16 (translated) */
    
    DWORD               DesiredAccess;
	DWORD               ShareMode;
	DWORD               FlagsAndAttributes;

	if (FileFlags & NeededFlags != FileFlags)
        ; //Fatal(ASSERT_NOT_ALLOWED_FILE_FLAGS, "Can not open file due to not allowed file flags");

	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

    /* Mapping from standard c library flags to windows flags 
	 * Here we match (Read and Write), Only read and only write */
    DesiredAccess = (FileFlags & O_RDWR) ? (GENERIC_WRITE | GENERIC_READ) :
					 ((FileFlags & O_WRONLY) ? GENERIC_WRITE : GENERIC_READ);
    
	/* Allow multiple processes to work with the file simultaneously or
	 * share the resource among processes. Otherwise when one process is 
	 * reading or modyfing the file, others will fail to open it */
	ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    
	/* Map file attributes */
	FlagsAndAttributes = 
		   FILE_ATTRIBUTE_NORMAL 
		 | ((FileFlags & O_RANDOM) ? FILE_FLAG_RANDOM_ACCESS : 0) 
		 | ((FileFlags & O_SEQUENTIAL) ? FILE_FLAG_SEQUENTIAL_SCAN : 0) 
		 | ((FileFlags & _O_SHORT_LIVED) ? FILE_ATTRIBUTE_TEMPORARY : 0) 
		 | ((FileFlags & O_TEMPORARY) ? FILE_FLAG_DELETE_ON_CLOSE : 0) 
		 | ((FileFlags & O_FILE_DIRECT) ? FILE_FLAG_NO_BUFFERING : 0) 
		 | ((FileFlags & _O_NOINHERIT) ? FILE_FLAG_WRITE_THROUGH : 0);

	/* In case of error, we try to open file again several times before giving up */
	while ((FileHandler = CreateFile(
		           FileName, 
				   DesiredAccess ,
				   ShareMode,
				   &sa,
				   MapCreationDisposition(FileFlags),
                   FlagsAndAttributes,
				   NULL)) == INVALID_HANDLE_VALUE)
	{
		/* The file can be locked by antivirus, firewall or backup process */
		DWORD		Error = GetLastError();
       
		if (Error == ERROR_PATH_NOT_FOUND)
            errno = FILE_PATH_NOT_FOUND;
		
		if (Error == ERROR_SHARING_VIOLATION ||
			Error == ERROR_LOCK_VIOLATION)
		{
			Sleep(100);
            i++;

	 		if (i < 300)
				continue;
		}
        
		//Log(ERR_CANNOT_OPEN_FILE, "Windows error code: %d", Error);

		return -1;
	}

	/* The _open_osfhandle function allocates a C run-time file descriptor 
	 * and associates it with the operating-system file handle specified by osfhandle. */
    if ((FileDescriptor = _open_osfhandle((intptr_t)FileHandler, FileFlags & O_APPEND)) < 0)
	{
		/* The file can be locked by antivirus, firewall or backup process */
		DWORD		Error = GetLastError();
		CloseHandle(FileHandler);
		return FileDescriptor;
	}
    
	/* The _setmode function sets to mode the translation mode of the file given by fd 
	 * _O_TEXT sets text (translated) mode. 
	 * _O_BINARY sets binary (untranslated) mode, in which these translations are suppressed */
    if (FileFlags & (O_TEXT | O_BINARY) && 
		_setmode(FileDescriptor, FileFlags & (O_TEXT | O_BINARY)) < 0)
	{
       _close(FileDescriptor);
		return -1;
	}

	return FileDescriptor;
}


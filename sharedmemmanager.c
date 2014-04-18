
#include "sharedmemmanager.h"

void*   sharMemSegAddr = NULL;
int     sharMemSegSize = 0;
HANDLE  sharMemSegId   = 0;

void*         sharMemStart;			
void*         sharMemEnd;			
SharMemHeader sharMemHdr;

#ifdef _WIN32

SharMemHeader SharMemCreate(
	void*           self,
	size_t          size, 
	Bool            makePrivate, 
	int             port)
{
	ISharedMemManager _    = (ISharedMemManager)self;
	IErrorLogger      elog = _->errorLogger;

	void*             mem;
	SharMemHeader     hdr;
	HANDLE		      hmap,
				      hmap2;

	char*             szSharMem;
	int			      i;
	DWORD		      sizeHigh;
	DWORD		      sizeLow;

	ASSERT(elog, size > ALIGN_DEFAULT(sizeof(SSharMemHeader)), False);

	szSharMem     = SHAR_MEM_NAME;
	sharMemSegAddr = NULL;

#ifdef _WIN64
	sizeHigh = size >> 32;
#else
	sizeHigh = 0;
#endif
	sizeHigh = (DWORD)size;

	/* When we create a file mapping and want to recycle
	 * the previous file we may get an error that the file 
	 * mapping object already exists. In this case we can 
	 * wait for 1 second and try repeat this 10 times.
	 */
	for (i = 0; i < 10; i++)
	{
		/* Clear the previous error code. */
		SetLastError(0);

		hmap = CreateFileMapping(
			       INVALID_HANDLE_VALUE,	/* Use the pagefile */
				   NULL,	                /* Default security attrs */
				   PAGE_READWRITE,		    /* Memory is Read/Write */
				   sizeHigh,	  	        /* Size Upper 32 Bits	*/
				   sizeLow,	        	    /* Size Lower 32 bits */
				   szSharMem);

		if (hmap == NULL)
			elog->log(LOG_ERROR, 
		          ERROR_CODE_CREATE_FILE_MAP_FAILED, 
				  "could not create file mapping: error code %lu (CreateFileMapping(size=%lu, name=%s))", 
				  GetLastError(),
				  size,
                  szSharMem);

		/* If the segment already existed, CreateFileMapping() 
		 * will return a handle to the existing one 
		 * and set ERROR_ALREADY_EXISTS.
		 */
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			/* Close the handle, since we got a valid one 
			 * to the previous segment.
			 */
			CloseHandle(hmap);	
								
			hmap = NULL;
			Sleep(1000);
			continue;
		}
		break;
	}

	/* If after having tried 10 times we still get the same error.
	 * It means that the shared memory exists and is in use.
	 * In this case we just report an error.
	 */
	if (hmap == NULL)
        elog->log(LOG_ERROR, 
		          ERROR_CODE_CREATE_FILE_MAP_FAILED, 
				  "Previous shared memory block still exists. Check if there are any other old processes and terminate them.");

	free(szSharMem);

	/* duplicate the shared memory handler and create another one
	 * which is inheritable.
	 */
	if (!DuplicateHandle(
		   GetCurrentProcess(), 
		   hmap, 
		   GetCurrentProcess(), 
		   &hmap2, 
		   0, 
		   TRUE, 
		   DUPLICATE_SAME_ACCESS))
	    elog->log(LOG_ERROR, 
		          ERROR_CODE_DUPLICATE_HANDLE_FAILED, 
				  "Could not duplicate shared memory handler.");
	
	/* Close and old non-inheritable handler. We do not 
	 * need it anymore so we can witout any doubts delete it.
	 */
	if (!CloseHandle(hmap))
        elog->log(LOG_ERROR, 
		          ERROR_CODE_CLOSE_HANDLER_FAILED, 
				  "could not close handle to backend parameter file: error code %lu",
				  GetLastError());

	/* Map the new memory to a local variable. */
	mem = MapViewOfFileEx(hmap2, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0, NULL);
	if (mem == NULL)
        elog->log(LOG_ERROR, 
		          ERROR_CODE_MAP_MEMORY_TO_FILE, 
				  "could not map backend parameter memory: error code %lu", 
				  GetLastError());

	hdr = (SharMemHeader)mem;
	hdr->procId     = GetProcessId(GetCurrentProcess());
	hdr->hdrId      = 0;
	hdr->totalSize  = size;
	hdr->freeoffset = ALIGN_DEFAULT(sizeof(SSharMemHeader));

	sharMemSegAddr = mem;
    sharMemSegSize = size;
    sharMemSegId   = hmap2;

	return hdr;
}

#endif

/* Set up basic pointers to shared memory. */
void initSharMemAccess(void* sharMem)
{
    SharMemHeader hdr = (SharMemHeader)sharMem;

    sharMemHdr   = hdr;
    sharMemStart = (void*)hdr;			
	sharMemEnd   = (char*)sharMemStart + hdr->totalSize;
}

void* ShmemInitStruct(
    void*        self,
	char*        name, 
	size_t       size, 
	Bool*        foundPtr)
{
    SharMemItem     result;
	void*           structPtr; 


} 
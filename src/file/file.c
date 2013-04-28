
#include "file.h"

#define     CLOSED_DESCRIPTOR              (-1)
#define     SYSTEM_RESERVED_FILE_IDS		10

/* The minimal number of allowed file descriptors
 * If the system can't open at least this number of file 
 * descriptors, we stop process */
#define     MIN_FILE_DESCRIPTORS			10

/* This parameter will be changed by DBA, because there are 
 * a lot of operating system which allows more opened files 
 * inside a process */
int			maxProcessFileDescriptors = 500;

/* Each operating system restricts the maximum number of opened 
 * file descriptors. It will be estimated inside Init() method */
int			maxNumberOfOpenedFileDescriptors = 64;	

typedef struct fileItem
{
	int			descriptor;		/* file pointer */
	long		seekPos;		/* position iside file */
	long		size;		    /* size of file */
	char*	    name;	  	    /* name of file. Memory for the file name 
								 * has been allocated using malloc function 
								 * and must be freed in the end of a function */	
	int			mode;		    /* mode of file */
	int		    nextFile;	    /* These items needed for two-directional list */
	int		    prevFile;
} FileItem;

/* The list of files and the list size */
static FileItem *Files;
static long SizeFiles = 0;
/* The difference between SizeFiles and FilesInUseNum is that SizeFiles
 * is the totals number of elements in Files array and some of them 
 * can be deleted. FilesInUseNum is the number of not deleted files */
static long	FilesInUseNum = 0;

void Init()
{
	/* Initialize files header */
    Files = (FileItem*)malloc(sizeof(FileItem));

	/* When malloc fails to aloocate memory */
	if (Files == NULL)
		Fatal(ERR_OUT_OF_MEMORY, "Can not allocate memory");

    /* Zero memory */
	memset((char*)Files, 0, sizeof(FileItem));

    SizeFiles = 1;
    DetermineMaxAllowedFileDescriptors();
}

/*
 * Counts how many file descriptors an operating system allows to open
 * and how many are already opened by other processes.
 * For example:
 * If the maxSystemAllowedDescriptors = 500 
 *        countAlreadyOpened = 40;
 * Then   countSystemMaxAllowed = maxSystemAllowedDescriptors - countAlreadyOpened = 460
 * If we reach max program allowed restrictions, we break the cycle
 */
static void CountMaxAllowedFiles(
	int maxProgramAllowed, 
	int* countSystemMaxAllowed, 
	int* countAlreadyOpened)
{ 
	int		   *FileDesctiptors;
	int			FileDesctiptorsSize = 1024;
	int			FileDesctiptorsCount = 0;
	int         MaxFileDescriptor = 0;
	int         j;
    
	/* Allocate initial memory for file descriptors array
	 * It is possible that we will neeed to extend it and allocate
	 * more memory. Thats why we allocate and allocate memory of 
	 * large chunks: 1024. */
    FileDesctiptors = (int*)malloc(FileDesctiptorsSize * sizeof(int));
    
	/* Duplicate zero file handler until we receive an exception 
	 * or exceed the max program allowed score */
    for (;;)
	{
		/* Standard dup function from c library calls the 
		 * standard windows (or analoguos function in Linux) DuplicateHandle
		 * These handlers are duplicated inside the current process and 
		 * do not affect other processes */
        int duplicatedFileId = dup(0);
        /* Negative handler indicates an error */
        if (duplicatedFileId < 0)
			break;
        /* If we have exceeded the allocated memory size we need to 
		 * add more memory */
		if (FileDesctiptorsCount >= FileDesctiptorsSize)
		{
			FileDesctiptorsSize *= 2;
			FileDesctiptors = 
				 (int*)realloc(FileDesctiptors, 
				               FileDesctiptorsSize * sizeof(int));
		}

		FileDesctiptors[FileDesctiptorsCount++] = duplicatedFileId;

		if (MaxFileDescriptor < duplicatedFileId)
			MaxFileDescriptor = duplicatedFileId;

		if (FileDesctiptorsCount >= maxProgramAllowed)
			break;
	}
	/* release the files we opened */
	for (j = 0; j < FileDesctiptorsCount; j++)
		close(FileDesctiptors[j]);

	free(FileDesctiptors);

	*countSystemMaxAllowed = FileDesctiptorsCount;
	/* Max file descriptor is the mamximum absolute number of 
	 * file descriptors inside the current operating system */
	*countAlreadyOpened = MaxFileDescriptor + 1 - FileDesctiptorsCount;
}

void DetermineMaxAllowedFileDescriptors()
{
	int         countSystemMaxAllowed;
	int			countAlreadyOpened;

	CountMaxAllowedFiles( 
		maxProcessFileDescriptors,
	    &countSystemMaxAllowed, 
	    &countAlreadyOpened);
    
	/* We won't exceed either max process file descriptors
	 * or experimentary defined file count limit */
    maxNumberOfOpenedFileDescriptors = 
		Min(countSystemMaxAllowed, 
		    maxProcessFileDescriptors - countAlreadyOpened);

	/* We need to subtract reserved for system amount */
	maxNumberOfOpenedFileDescriptors -= SYSTEM_RESERVED_FILE_IDS;

	if (maxNumberOfOpenedFileDescriptors < MIN_FILE_DESCRIPTORS)
        Fatal(ERR_INSUFFICIENT_FILE_DESCRIPTORS, 
		      "%s. The max allowed value is: %d. The minimal needed is: %d",
			  "Insufficient number of max allowed file descriptors",
			  maxNumberOfOpenedFileDescriptors + SYSTEM_RESERVED_FILE_IDS,
			  MIN_FILE_DESCRIPTORS + SYSTEM_RESERVED_FILE_IDS);
}

int FileOpen(char* FileName, int FileFlags, int FileMode)
{
	int FileDescriptor;

	FileDescriptor = OsFileOpen(FileName, FileFlags, FileMode);

	if (FileDescriptor >= 0)
		return FileDescriptor;	

    /* EMFILE means too many open files. 
	 * ENFILE means the same, too many open files  */
    if (errno == EMFILE || errno == ENFILE)
	    Log(ERR_TOO_MANY_OPEN_FILES, "There are too many opened files in system");

    return -1;
}

/* This function simply delete file from two-directional system */
static void DeleteFromList(int FileDescriptor)
{    
	FileItem* Current = &Files[FileDescriptor];

	Files[Current->PrevFile].NextFile = Current->NextFile;
	Files[Current->NextFile].PrevFile = Current->PrevFile;
}

static void Delete(int FileDescriptor)
{
    FileItem* Current = &Files[FileDescriptor];

	DeleteFromList(FileDescriptor);
    /* Save current seek position */
	Current->seekPos = lseek(Current->descriptor, (long)0, SEEK_CUR);

	/* Close the file */
	if (close(Current->descriptor))
		Log(ERR_TOO_MANY_OPEN_FILES, "Could not close file %s", Current->name);

	FilesInUseNum--;
	Current->descriptor = CLOSED_DESCRIPTOR;
}

static void InsertIntoList(int FileDescriptor)
{
	FileItem* Current = &Files[FileDescriptor];

	/* Insert in the end of the list. The next pointer of the last 
	 * item in the list points to 0th element */
	Current->nextFile = 0;
	Current->prevFile = Files[0].prevFile;

	Files[0].prevFile = FileDescriptor;
	Files[Files[0].prevFile].nextFile = FileDescriptor;
}

static int Insert(int FileDescriptor)
{
	FileItem* Current = &Files[FileDescriptor];

	if (Current->descriptor == CLOSED_DESCRIPTOR)
	{
		while (nfile + numAllocatedDescs >= max_safe_fds)
		{
			if (!ReleaseLruFile())
				break;
		}

		/*
		 * The open could still fail for lack of file descriptors, eg due to
		 * overall system file table being full.  So, be prepared to release
		 * another FD if necessary...
		 */
		vfdP->fd = BasicOpenFile(vfdP->fileName, vfdP->fileFlags,
								 vfdP->fileMode);
		if (vfdP->fd < 0)
		{
			DO_DB(elog(LOG, "RE_OPEN FAILED: %d", errno));
			return vfdP->fd;
		}
		else
		{
			DO_DB(elog(LOG, "RE_OPEN SUCCESS"));
			++nfile;
		}

		/* seek to the right position */
		if (vfdP->seekPos != (off_t) 0)
		{
			off_t returnValue PG_USED_FOR_ASSERTS_ONLY;

			returnValue = lseek(vfdP->fd, vfdP->seekPos, SEEK_SET);
			Assert(returnValue != (off_t) -1);
		}
	}

	InsertIntoList(FileDescriptor);

	return 0;
}


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
	int         nextFreePosition; 
	int         flags;
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

	Files[Current->prevFile].nextFile = Current->nextFile;
	Files[Current->nextFile].prevFile = Current->prevFile;
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
/* If there are no files in use, the function retirns false.
 * If there is at least one file in use we delete the next file after 0
 * Our list has the structure: short-term stayed files <- 0 -> long-term stayed files
 */
static int ClearFileList()
{
	if (FilesInUseNum > 0)
	{   
		if (Files[0].nextFile != 0)
		    Delete(Files[0].nextFile);
		return 1;			
	}
	return 0;				
}

static int Insert(int FileDescriptor)
{
	FileItem* Current = &Files[FileDescriptor];

	if (Current->descriptor == CLOSED_DESCRIPTOR)
	{
		/* We have exceeded the maximum allowed number of opened files
		 * We delete the oldest files from the ring until reach allowed amount */
		while (FilesInUseNum >= maxNumberOfOpenedFileDescriptors)
		{
			if (!ReleaseLruFile())
				break;
		}

		Current->descriptor = FileOpen(Current->name, Current->flags, Current->mode);
        /* re open the file failed */
		if (Current->descriptor < 0)
			return Current->descriptor;
		else
			++FilesInUseNum; /* re-open was succesful */

		/* come back to the previous position inside file */
		if (Current->seekPos != (long)0)
			lseek(Current->descriptor, Current->seekPos, SEEK_SET);
	}

	InsertIntoList(FileDescriptor);

	return 0;
}

static long AllocateVfd()
{
	int		i;
	int		fileDescriptor;

	if (VfdCache[0].nextFree == 0)
	{
		/*
		 * The free list is empty so it is time to increase the size of the
		 * array.  We choose to double it each time this happens. However,
		 * there's not much point in starting *real* small.
		 */
		Size		newCacheSize = SizeVfdCache * 2;
		Vfd		   *newVfdCache;

		if (newCacheSize < 32)
			newCacheSize = 32;

		/*
		 * Be careful not to clobber VfdCache ptr if realloc fails.
		 */
		newVfdCache = (Vfd *) realloc(VfdCache, sizeof(Vfd) * newCacheSize);
		if (newVfdCache == NULL)
			ereport(ERROR,
					(errcode(ERRCODE_OUT_OF_MEMORY),
					 errmsg("out of memory")));
		VfdCache = newVfdCache;

		/*
		 * Initialize the new entries and link them into the free list.
		 */
		for (i = SizeVfdCache; i < newCacheSize; i++)
		{
			MemSet((char *) &(VfdCache[i]), 0, sizeof(Vfd));
			VfdCache[i].nextFree = i + 1;
			VfdCache[i].fd = VFD_CLOSED;
		}
		VfdCache[newCacheSize - 1].nextFree = 0;
		VfdCache[0].nextFree = SizeVfdCache;

		/*
		 * Record the new size
		 */
		SizeVfdCache = newCacheSize;
	}

	file = VfdCache[0].nextFree;

	VfdCache[0].nextFree = VfdCache[file].nextFree;

	return file;
}
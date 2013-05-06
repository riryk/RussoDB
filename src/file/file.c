
#include "file.h"

#define     CLOSED_DESCRIPTOR              (-1)
#define     SYSTEM_RESERVED_FILE_IDS		10
#define     PATH_MAX_LENGTH                 1024

#define     TABLE_SPACE_DEFAULT_ID          5000
#define     TABLE_SPACE_GLOBAL_ID           5001

/* The minimal number of allowed file descriptors
 * If the system can't open at least this number of file 
 * descriptors, we stop process */
#define     MIN_FILE_DESCRIPTORS			10

#define     FILE_DELETE_WHEN_CLOSED		   (1 << 0)
#define     DELETE_TRANSACTION_END	       (1 << 1)	

unsigned int  CurrentDatabaseTableSpace = 0;

/* Current process identifier */
int         currentProcessId;                

/* This parameter will be changed by DBA, because there are 
 * a lot of operating system which allows more opened files 
 * inside a process */
int			maxProcessFileDescriptors = 500;

/* Each operating system restricts the maximum number of opened 
 * file descriptors. It will be estimated inside Init() method */
int			maxNumberOfOpenedFileDescriptors = 64;	

/* Each transaction can have a lot of temporary table spaces
 * If the count of temporary tablespaces is -1, that means that it
 * has not been set by current transaction */
static int	temporaryTableSpaceCount = -1;

/* A pointer to the next table space */
static int  nextTempTableSpace = 0;

/* A list of temporary spaces */
static unsigned int *temporarySpacesArray = NULL;

/* The number of opened temp files */
static long tempFilesCount = 0;

static int HasCurrentTranTempFiles = 0;

/* Total temporary files size */
static uint64 TempFilesSize = 0;

static int TempFileSizeLimit = -1;

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
	unsigned short state;	    /* A state of the file */
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

int FileOpenBase(char* FileName, int FileFlags, int FileMode)
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
			if (! ClearFileList())
				break;
		}

		Current->descriptor = ROpenFile(Current->name, Current->flags, Current->mode);
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

/* Allocates new memory for the Files array */
static int GetNextFreeItem()
{
	int		i;
	int		fileDescriptor;

	/* When we have noticed that there are no free plates inside 
	 * Files array we need to reallocate memory for that. We simply 
	 * double it */
	if (Files[0].nextFreePosition == 0)
	{	
		long		newFilesSize = SizeFiles * 2;
	    FileItem	*newFiles;

		if (newFilesSize < 32)
			newFilesSize = 32;
		/* Reallocate new  */
		newFiles = (FileItem*)realloc(newFiles, sizeof(FileItem) * newFilesSize);
		if (newFiles == NULL)
            Fatal(ERR_OUT_OF_MEMORY, "Can not allocate memory");

		Files = newFiles;

		/* Add new items into free list */
		for (i = SizeFiles; i < newFilesSize; i++)
		{
			memset((char*)&(Files[i]), 0, sizeof(FileItem));
			Files[i].nextFreePosition = i + 1;
			Files[i].descriptor = CLOSED_DESCRIPTOR;
		}

		Files[newFilesSize - 1].nextFreePosition = 0;
		Files[0].nextFreePosition = SizeFiles;

		Files = newFiles;
	}
    /* Get the first free node in Files array to client */
	fileDescriptor = Files[0].nextFreePosition;
	Files[0].nextFreePosition = Files[fileDescriptor].nextFreePosition;

	return fileDescriptor;
}

/* Mark file as deleted inside Files list */
void FreeFile(int fileDescriptor)
{
	FileItem	   *file = &Files[fileDescriptor];

	if (file->name != NULL)
	{
		free(file->name);
		file->name = NULL;
	}
    /* Insert this cell into free list */
	file->nextFreePosition = Files[0].nextFreePosition;
	Files[0].nextFreePosition = file;
}

static int FileReOpen(int fileDescriptor)
{
	if (Files[fileDescriptor].descriptor == CLOSED_DESCRIPTOR)
	{
		int returnValue = Insert(fileDescriptor);
		if (returnValue != 0)
			return returnValue;
		return 0;
	}

	/* Move file to the head of the list */
	if (Files[0].prevFile != fileDescriptor)
	{
		DeleteFromList(fileDescriptor);
		InsertIntoList(fileDescriptor);
	}

	return 0;
}

int ROpenFile(char* FileName, int FileFlags, int FileMode)
{
	char*       FileNameCopy;
	int		    FileHandler;
	FileItem*   Item;

	/* We need to copy FileName. If strdup function returns null
	 * there can only be if there are no free memory in the system */
	FileNameCopy = strdup(FileName);
	if (FileNameCopy == NULL)
		Fatal(ERR_OUT_OF_MEMORY, "Can not allocate memory");
    
	/* Get free descriptor from Files list */
	FileHandler = GetNextFreeItem();
	Item = &Files[FileHandler];

	/* Get free position inside the list */
	while (FilesInUseNum >= maxNumberOfOpenedFileDescriptors)
    {
	   if (!ClearFileList())
		  break;
    }

	Item->descriptor = FileOpenBase(FileName, FileFlags, FileMode);
    /* If some error has happened we free this descriptor and FileNameCopy */
	if (Item->descriptor < 0)
	{
		FreeFile(FileHandler);
		free(FileNameCopy);
		return -1;
	}

	++FilesInUseNum;

	Insert(FileHandler);

	Item->name = FileNameCopy;
	Item->flags = FileFlags & ~(O_CREAT | O_TRUNC | O_EXCL);
	Item->mode = FileMode;
	Item->seekPos = 0;
	Item->size = 0;

	return FileHandler;
}

/* This function returns returns 0 when there are no temp table spaces */
unsigned int GetNextTempTableSpace()
{
	if (temporaryTableSpaceCount > 0)
	{
		/* If we reach the end of the temporary array, 
		 * we come back to the beginning */
		if (++nextTempTableSpace >= temporaryTableSpaceCount)
			nextTempTableSpace = 0;

		return temporarySpacesArray[nextTempTableSpace];
	}
	return 0;
}

static int OpenTempFileInTablespace(int tableSpaceId, int error)
{
	char		DirectoryPath[PATH_MAX_LENGTH];
	char		FilePath[PATH_MAX_LENGTH];
	int 		fileDescriptor;

	if (tableSpaceId == TABLE_SPACE_DEFAULT_ID || tableSpaceId == TABLE_SPACE_GLOBAL_ID)
	    _snprintf_s(DirectoryPath, sizeof(DirectoryPath), 1024, "DefaultTableSpace/Temp");
	else
		_snprintf_s(DirectoryPath, sizeof(DirectoryPath), 1024, "TableSpaces/%u/Temp", tableSpaceId);

	/* Generate temp file name */
	_snprintf_s(FilePath, sizeof(FilePath), 1024, "%s/Temp%d.%ld",
			    DirectoryPath, currentProcessId, tempFilesCount++);

	fileDescriptor = OpenFile(FilePath, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, 0600);

	if (fileDescriptor <= 0)
	{
		/* In case of error try to create  */
		mkdir(DirectoryPath, _S_IREAD | _S_IWRITE | _S_IEXEC);
        
		/* Try to open again after creating  */
		fileDescriptor = OpenFile(FilePath, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, 0600);

		if (fileDescriptor <= 0 && error)
			Log(ERR_COULD_NOT_CREATE_TEMP_FILE, "could not create temporary file \"%s\"", FilePath);
	}

	return fileDescriptor;
}

/* if crossTransactions flag is set to true - this file is supposed 
 * to live more than the current transaction. We save it into 
 * default database tablespace. Temporary tablespaces can easily be deleted 
 * by other transactions. */
int static OpenTempFile(int crossTransactions)
{
	int		fileDescriptor = 0;

	/* If the current transaction has given us some tablespaces 
	 * we save this file into one of them */
	if (temporaryTableSpaceCount > 0 && !crossTransactions)
	{
		unsigned int   tableSpaceId = GetNextTempTableSpace();

		if (tableSpaceId != 0)
			fileDescriptor = OpenTempFileInTablespace(tableSpaceId, 0);
	}

	if (fileDescriptor <= 0)
	{
		int TableSpace = CurrentDatabaseTableSpace ? 
                         CurrentDatabaseTableSpace :
                         TABLE_SPACE_DEFAULT_ID;

		fileDescriptor = OpenTemporaryFileInTablespace(TableSpace, 1);
	}

	Files[fileDescriptor].state |= FILE_DELETE_WHEN_CLOSED;

	if (!crossTransactions)
	{
		Files[fileDescriptor].state |= DELETE_TRANSACTION_END;

		//ResourceOwnerEnlargeFiles(CurrentResourceOwner);
		//ResourceOwnerRememberFile(CurrentResourceOwner, file);
		//VfdCache[file].resowner = CurrentResourceOwner;

		HasCurrentTranTempFiles = 1;
	}

	return fileDescriptor;
}

void CloseFile(int fileDescriptor)
{
	FileItem*    file = &Files[fileDescriptor];
    
	if (file->descriptor == CLOSED_DESCRIPTOR)
	{
        Delete(file->descriptor);
	}
  
	if (file->descriptor == CLOSED_DESCRIPTOR) 
	{
		Delete(file);

		if (close(file->descriptor))
			Log("could not close file \"%s\"", file->name);

		FilesInUseNum--;
		file->descriptor = CLOSED_DESCRIPTOR;
	}

	/* If the file is temporary, we delete it */
	if (file->state & FILE_DELETE_WHEN_CLOSED)
	{
		struct stat    FileStatistic;
		int			   Error;

		file->state &= ~FILE_DELETE_WHEN_CLOSED;
		TempFilesSize -= file->size;

		file->size = 0;

		if (stat(file->name, &FileStatistic))
			Error = errno;
		else
			Error = 0;

		if (unlink(file->name))
            Log("could not unlink file \"%s\"", file->name);

		if (Error == 0)
		{
			// Write to log file
		}
		else
		{
			errno = Error;
			Log("could not stat file \"%s\"", file->name);
		}
	}

	/* Free the file slot */
	FreeFile(fileDescriptor);
}

int RReadFile(int FileDescriptor, char *Buffer, int Amount)
{
	int			returnCode;
    int         ContinueReading = 1;

	returnCode = FileReOpen(FileDescriptor);
	if (returnCode < 0)
		return returnCode;

    while (ContinueReading)
	{
		returnCode = read(Files[FileDescriptor].descriptor, Buffer, Amount);
        ContinueReading = 0;

		if (returnCode >= 0)
			Files[FileDescriptor].seekPos += returnCode;
		else
		{
			DWORD		error = GetLastError();

			switch (error)
			{
				case ERROR_NO_SYSTEM_RESOURCES:
					Sleep(1000);
					errno = ERR_INSUFFICIENT_SYSTEM_RESOURCE;
                    ContinueReading = 1;
					break;
				default:
					Log("unexpected error has happened \"%s\"", error);
					break;
			}

            if (ContinueReading == 1)
				continue;

			Files[FileDescriptor].seekPos = -1;
		}
	}

	return returnCode;
}

int FileWrite(int FileDescriptor, char* Buffer, int Amount)
{
	int			returnCode;
	int         ContinueWriting = 1;

	returnCode = FileReOpen(FileDescriptor);
	if (returnCode < 0)
		return returnCode;

	if (TempFileSizeLimit >= 0 && (Files[FileDescriptor].descriptor & FILE_DELETE_WHEN_CLOSED))
	{
		long		NewPos = Files[FileDescriptor].seekPos + Amount;

		if (NewPos > Files[FileDescriptor].size)
		{
			uint64		newTotal = TempFilesSize;

			newTotal += NewPos - Files[FileDescriptor].size;
			if (newTotal > (uint64) TempFileSizeLimit * (uint64) 1024)
                Fatal(ERR_TEMP_FILES_LIMIT_EXCEEDED, 
				      "temporary file size exceeds TempFileSizeLimit (%dkB)",
					  TempFileSizeLimit);
		}
	}

    while (ContinueWriting)
	{
		errno = 0;
		returnCode = write(Files[FileDescriptor].descriptor, Buffer, Amount);
		ContinueWriting = 0;

		/* if write didn't set errno, assume problem is no disk space */
		if (returnCode != Amount && errno == 0)
			errno = ENOSPC;

		if (returnCode >= 0)
		{
			Files[FileDescriptor].seekPos += returnCode;

			/* If this file is a temporary file, we recalculate tempfilesize */
			if (Files[FileDescriptor].state & FILE_DELETE_WHEN_CLOSED)
			{
				long    newPosition = Files[FileDescriptor].seekPos;

				if (newPosition > Files[FileDescriptor].size)
				{
					TempFilesSize += newPosition - Files[FileDescriptor].size;
					Files[FileDescriptor].size = newPosition;
				}
			}
		}
		else
		{
			DWORD		error = GetLastError();

			switch (error)
			{
				case ERROR_NO_SYSTEM_RESOURCES:
					Sleep(1000);
					errno = EINTR;
					ContinueWriting = 1;
					break;
				default:
					Log("unexpected error has happened \"%s\"", error);
					break;
			}

			if (ContinueWriting == 1)
				continue;

			Files[FileDescriptor].seekPos = -1;
		}
	}

	return returnCode;
}

int FileSync(int FileDescriptor)
{
	int			returnCode;

	returnCode = FileReOpen(FileDescriptor);
	if (returnCode < 0)
		return returnCode;

	_commit(Files[FileDescriptor].descriptor);
}

long FileSeek(int FileDescriptor, long Offset, int Mode)
{
	int			returnCode;

	if (Files[FileDescriptor].descriptor = CLOSED_DESCRIPTOR)
	{
		switch (Mode)
		{
			case SEEK_SET:
				if (Offset < 0)
					Log("invalid seek offset %d", Offset);
				Files[FileDescriptor].seekPos = Offset;
				break;
			case SEEK_CUR:
				Files[FileDescriptor].seekPos + Offset;
				break;
			case SEEK_END:
				returnCode = FileReOpen(FileDescriptor);
				if (returnCode < 0)
					return returnCode;
				Files[FileDescriptor].seekPos = 
					lseek(Files[FileDescriptor].descriptor, Offset, Mode);
				break;
			default:
				Log("invalid Mode %d", Mode);
				break;
		}
	}
	else
	{
		switch (Mode)
		{
			case SEEK_SET:
				if (Offset < 0)
					Log("invalid seek offset %d", Offset);
				if (Files[FileDescriptor].seekPos != Offset)
					Files[FileDescriptor].seekPos = 
					   lseek(Files[FileDescriptor].descriptor, Offset, Mode);
				break;
			case SEEK_CUR:
				if (Offset != 0 || Files[FileDescriptor].seekPos == -1)
					Files[FileDescriptor].seekPos = 
					   lseek(Files[FileDescriptor].descriptor, Offset, Mode);
				break;
			case SEEK_END:
				Files[FileDescriptor].seekPos = 
					   lseek(Files[FileDescriptor].descriptor, Offset, Mode);
				break;
			default:
			    Log("invalid seek offset %d", Offset);
				break;
		}
	}
	return Files[FileDescriptor].seekPos;
}
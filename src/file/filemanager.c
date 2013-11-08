
#include "filemanager.h"

const SIFileManager sFileManager = 
{ 
	openFile,
    restoreFilePos
};

const IFileManager fileManager = &sFileManager;

int getFreeCachePosition()
{
	int         i;
	int		    file;

	/* nextFree == 0 means that the free list is empty.
	 * We need to allocate new items to our file descriptors array.
	 * We double it each time. But the size should not be less than 32.
	 */
	if (fileCache[0].nextFree == 0)
	{
        size_t	       newSize = sizeFileCache * 2;
		FileCacheItem  newCache;

		if (newSize < 32)
			newSize = 32;

		fileCache = (FileCacheItem)realloc(fileCache, sizeof(SFileCacheItem) * newSize);

		for (i = sizeFileCache; i < newSize; i++)
		{
			memset((char*)&(fileCache[i]), 0, sizeof(SFileCacheItem));
			fileCache[i].nextFree = i + 1;
			fileCache[i].fileDesc = FILE_CLOSED;
		}
	
		fileCache[newSize - 1].nextFree = 0;
		fileCache[0].nextFree = sizeFileCache;

		sizeFileCache = newSize;
	}

	file = fileCache[0].nextFree;

	fileCache[0].nextFree = fileCache[file].nextFree;

	return file;
}

Bool deleteFileCache()
{
	if (fileCount > 0)
	{
		int            fileInd  = fileCache[0].moreRecent;
        FileCacheItem  fileItem = &fileCache[fileInd];

		fileCache[fileItem->lessRecent].moreRecent = fileItem->moreRecent;
		fileCache[fileItem->moreRecent].lessRecent = fileItem->lessRecent;

		/* Save the current file pointer position. If the mode is SEEK_CUR
		 * we add position to the current position.
		 */
		fileItem->seekPos = lseek(fileItem->fileDesc, 0, SEEK_CUR);
		close(fileItem->fileDesc);

		fileCount--;
		return True;
	}
	return False;
}

void insertIntoFileCache(int fileInd)
{
	FileCacheItem  fileItem = &fileCache[fileInd]; 

	fileItem->moreRecent = 0;
	fileItem->lessRecent = fileCache[0].lessRecent;

	fileCache[0].lessRecent = fileInd;
	fileCache[fileItem->lessRecent].moreRecent = fileInd;
}

int insertIntoArray(int fileInd)
{
    FileCacheItem  fileItem = &fileCache[fileInd];

    if (fileItem->fileDesc == FILE_CLOSED)
	{
        while (fileCount + allocatedDescsCount >= maxFileDescriptors)
	    {
            if (!deleteFileCache())
		        break;
	    }

        fileItem->fileDesc = openFileBase(
		                           fileItem->name, 
		                           fileItem->flags, 
								   fileItem->mode);
	    if (fileItem->fileDesc < 0)
            return fileItem->fileDesc;
	    
        fileCount++; 
		if (fileItem->seekPos != 0)
			lseek(fileItem->fileDesc, fileItem->seekPos, SEEK_SET);
	}

    insertIntoFileCache(fileInd);
}

int openFileBase(char* fname, int fileFlags, int fileMode)
{
	int       fileDesc;
 
	while (1 == 1)
	{
        fileDesc = fileOpen(fname, fileFlags, fileMode);

	    if (fileDesc >= 0)
		    return fileDesc;
        
        if (!(errno == EMFILE || errno == ENFILE))
			return -1;

        if (deleteFileCache())
		    continue;

		return -1;
	}
	return -1;
}

void removeFileToFreeList(int fileInd)
{
    FileCacheItem  fileItem = &fileCache[fileInd]; 

	if (fileItem->name != NULL)
	{
		free(fileItem->name);
		fileItem->name = NULL;
	}

	fileItem->state = 0;

	fileItem->nextFree    = fileCache[0].nextFree;
    fileCache[0].nextFree = fileInd;
}

int changeFileArrayPos(int fileId)
{
    if (fileCache[fileId].fileDesc == FILE_CLOSED)
	{
        int retValue = insertIntoArray(fileId);
		if (retValue)
			return retValue;
		return 0;
	}

	if (fileCache[0].lessRecent != fileId)
	{
        removeFileToFreeList(fileId);
        insertIntoArray(fileId);
	}
	return 0;
}

long restoreFilePos(int fileId, long offset, int placeToPut)
{
	if (fileCache[fileId].fileDesc == FILE_CLOSED)
	{
        if (placeToPut == SEEK_SET)
			fileCache[fileId].seekPos = offset;
		if (placeToPut == SEEK_CUR)
            fileCache[fileId].seekPos += offset;
		if (placeToPut == SEEK_END)
		{
            int retCode = changeFileArrayPos(fileId);
            if (retCode < 0)
				return retCode;
			fileCache[fileId].seekPos = lseek(fileCache[fileId].fileDesc, offset, placeToPut);
		}
		return fileCache[fileId].seekPos;
	}
	if (placeToPut == SEEK_SET && fileCache[fileId].seekPos != offset)
		fileCache[fileId].seekPos = lseek(fileCache[fileId].fileDesc, offset, placeToPut);
	if (placeToPut == SEEK_CUR && (offset != 0 || fileCache[fileId].seekPos == -1))
        fileCache[fileId].seekPos = lseek(fileCache[fileId].fileDesc, offset, placeToPut);
	if (placeToPut == SEEK_END)
        fileCache[fileId].seekPos = lseek(fileCache[fileId].fileDesc, offset, placeToPut);

	return fileCache[fileId].seekPos;
}

int openFile(char* fname, int fileFlags, int fileMode)
{
    char*          fnameCopy = strdup(fname);
    int            fileInd;
	FileCacheItem  fileItem;

    fileInd  = getFreeCachePosition();
    fileItem = &fileCache[fileInd]; 

	while (fileCount + allocatedDescsCount >= maxFileDescriptors)
	{
        if (!deleteFileCache())
		    break;
	}

	fileItem->fileDesc = openFileBase(fname, fileFlags, fileMode);
	if (fileItem->fileDesc < 0)
	{
		removeFileToFreeList(fileInd);
        free(fnameCopy);
	    return -1;
	}

    fileCount++;
    insertIntoFileCache(fileInd);

	fileItem->name = fnameCopy;
	fileItem->flags = fileFlags & ~(_O_CREAT | _O_TRUNC | _O_EXCL); 
	fileItem->mode = fileMode;
	fileItem->seekPos = 0;
	fileItem->size = 0;
	fileItem->state = 0;

	return fileInd;
} 
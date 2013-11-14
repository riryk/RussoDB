
#include "filemanager.h"

const SIFileManager sFileManager = 
{ 
	&sTrackMemManager,
    ctorFileMan,
	openFileToCache,
    restoreFilePos,
    cacheInsert,
    cacheRealloc,
    cacheGetFree,
	cacheDelete,
    closeRecentFile,
	openFile
};

const IFileManager fileManager = &sFileManager;

int		         fileMaxCount   = 32;

FCacheEl         fileCache      = NULL;
size_t           fileCacheCount = 0;
int	             fileCount      = 0;



void ctorFileMan(void* self)
{
    IFileManager   _      = (IFileManager)self;
	IMemoryManager memMan = _->memManager;

	fileCache = (FCacheEl)memMan->alloc(sizeof(SFCacheEl));

	memset((char*)&(fileCache[0]), 0, sizeof(SFCacheEl));
	fileCache->fileDesc = FILE_INVALID;

	fileCacheCount = 1;
}

/* Allocates new items to the file cache array.
 *
 * 1. From the start 
 * we have only one item fileCache[0]
 * and nextFree points to 0-th item:   0-\   picture
 *                                     ^  |
 *                                      \/ 
 * Then we allocate new 31 items, numerated from 1 to 31
 * which are arranged into one-directed linked list:
 *
 *  1->2->3->3->....->30->31
 *
 * Then we add this linked list into our single self-connected ring
 * 
 *  0 -> 1 -> 2 -> 3 -> .... -> 30 -> 31-\
 *  ^                                    |
 *   \----------------------------------/
 *
 */
void cacheRealloc(void* self)
{
	IFileManager   _        = (IFileManager)self;
	IMemoryManager memMan   = _->memManager;

    size_t	       newCount = fileCacheCount * 2;
    int            min      = MIN_CACHE_EL_TO_ALLOC;
	FCacheEl       newCache;
	int            i;

	if (newCount < min)
		newCount = min;

	fileCache = memMan->realloc(fileCache, sizeof(SFCacheEl) * newCount);

	for (i = fileCacheCount; i < newCount; i++)
	{
		memset((char*)&(fileCache[i]), 0, sizeof(SFCacheEl));

		/* ith nextFree pointer points to the next item */
		fileCache[i].nextFree = i + 1;
		fileCache[i].fileDesc = FILE_INVALID;
	}
    /* The last item in the new array points to the head, 0 item */
	fileCache[newCount - 1].nextFree = 0;

	/* We have just added new free items to the end 
	 * of the file cache array and the head's next free pointer
	 * should point to the first position of the free list.
	 */
	fileCache[0].nextFree = fileCacheCount;

	fileCacheCount = newCount;
}

/* Removes the first item from the free list.
 * The first item from the free list is fileCache[0]->nextFree
 * We remove it and set fileCache[0]->nextFree to the
 * second free item.
 * If there are no free items left we reallocate 
 * the cache array.
 * After we have removed the first 3 items the array looks like:
 *    -------------
 *   /             \
 *  /               v
 * 0  1 -> 2 -> 3 -> 4 -> .... -> 30 -> 31-\
 *  ^                                      |
 *   \------------------------------------/
 * So that zero item points to item 4
 */
int cacheGetFree(void* self)
{
	IFileManager _  = (IFileManager)self;
	int          i;
	int		     file;

	/* nextFree == 0 means that the free list is empty.
	 * We need to allocate new items to our file descriptors array.
	 * We double it each time. But the size should not be less than 32.
	 */
	if (fileCache[0].nextFree == 0)
	    _->cacheRealloc(_);

	file = fileCache[0].nextFree;
	fileCache[0].nextFree = fileCache[file].nextFree;

	return file;
}

/*  There is a restriction about how many file handlers 
 * can open one process in an operation system. 
 * fileMaxCount = 32 is a default value of max allowed 
 * opened file handlers. 
 *  We should estimate this count 
 * because there are different systems and some of file handler
 * can be already opened.
 *  If we reach the maximum allowed file descriptors count we should 
 * release operation system resources and close at least one file handler.
 *  Choose for example one recent file. This means that it has been opened  
 * not very long ago and is being actively used. So there is a great possibility
 * that our process will want to use this file descriptor again. Reopening a
 * file handler is very expensive operation. So we would like to cache it.
 *  The rightest idea is to close the most recent file, because probably 
 * our process do not longer use it.
 */
Bool closeRecentFile(void*     self)
{   
	IFileManager _  = (IFileManager)self;
	int          theMostRecentInd;
    FCacheEl     theMostRecent;
	int          fd;

	if (fileCount == 0)
		return False;

    theMostRecentInd = fileCache[0].moreRecent;
	theMostRecent    = &fileCache[theMostRecentInd];
	fd               = theMostRecent->fileDesc;

	_->cacheDelete(theMostRecentInd);

    /* Save the current file pointer position. If the mode is SEEK_CUR
	 * we add position to the current position.
	 */
	theMostRecent->seekPos = lseek(fd, 0, SEEK_CUR);
	close(fd);

	fileCount--;
	return True;
}

/* This method deletes an item from the ring.
 * 
 *            lessRecent            lessRecent     
 *... prev <------------ current <------------ next ...
 *        \  moreRecent  ^      \ moreRecent  ^ 
 *         \------------/        \-----------/  
 *
 * After deleting the ring looks like:
 * 
 *                 lessRecent     
 *     ... prev <------------ next 
 *             \  moreRecent  ^  
 *              \------------/   
 */
void cacheDelete(int pos)
{
    FCacheEl   theCurrent = &fileCache[pos];

    int        theNextInd = theCurrent->moreRecent;
	int        thePrevInd = theCurrent->lessRecent;

	FCacheEl   theNext = &fileCache[theNextInd];
	FCacheEl   thePrev = &fileCache[thePrevInd];

	thePrev->moreRecent = theNextInd;
	theNext->lessRecent = thePrevInd;
}

/* Our file cache consists of two-directional list of 
 * file cache items sorted by the time it have been opened.
 * Each cache item has links: lessRecent and moreRecent.
 * 1. From start we have:
 *  lessRecent    /- 0--\ moreRecent
 *               |   ^  | 
 *                \/  \/ 
 * 2. Insert one position, pos = 5 into this ring:
 *             moreRecent
 *         /-----------\
 *        /             |
 *       V   lessRecent |    
 *      0 <------------ 5            6
 *      |\   lessRecent ^^
 *      | \------------/  \
 *       \----------------/
 *           moreRecent
 *
 *  After we have inserted the first position 
 *  into our time opening list we have that for our header 0
 *  moreRecent and lessRecent pointers point to the first position 5.
 *  And moreRecent and lessRecent for 5 point to the header 0.
 *  
 * 3. Insert the second position to the ring.  
 *                lessRecent
 *       /----------------------------\
 *      |                              |
 *      |   lessRecent     lessRecent  V
 *      0 <----------- 5 <------------ 6       7
 *      ^\ moreRecent^ |  moreRecent ^ |
 *      | \----------/  \-----------/  |
 *      |    moreRecent                |
 *       \----------------------------/
 *
 * 4. So we can figure out an algorithm to insert a new item to 
 *    our ring.
 *   theHead = 0, theMostRecent = 6, theNewItem = 7
 *   Tie the new item with the head:
 *              theHead.lessRecent = theNewItem, 
 *              theNewItem.moreRecent = theHead.
 *   Tie the new item with the previous the most recent item:
 *              theMostRecent.moreRecent = theNewItem,
 *              theNewItem.lessRecent = theMostRecent.
 *  
 * 5. More convenient way to represent the ring:
 *        lessRecent      lessRecent     lessRecent
 *    0 <------------ 5 <------------ 6 <----------- 0
 *     \  moreRecent  ^\ moreRecent  ^ \ moreRecent  ^
 *      \------------/  \-----------/   \-----------/
 * 
 * So that the least recent file is head->lessRecent
 * and the most recent file is head->moreRecent.
 */ 
void cacheInsert(int pos)
{
	int      theHeadIndex      = 0;
    FCacheEl theHead           = &fileCache[0]; 

	int      theLeastRecentInd = theHead->lessRecent; 
	FCacheEl theLeastRecent    = &fileCache[theLeastRecentInd]; 

	FCacheEl theNewItem        = &fileCache[pos]; 
    
	/* Tie the new item with the least recent item */
	theNewItem->lessRecent     = theLeastRecentInd;
	theLeastRecent->moreRecent = pos;
    
	/* Tie the new item with the head */
	theHead->lessRecent    = pos;
	theNewItem->moreRecent = 0;
}

int insertIntoArray(void* self, int fileInd)
{
    FCacheEl  fileItem = &fileCache[fileInd];

    if (fileItem->fileDesc == FILE_INVALID)
	{
        while (fileCount >= fileMaxCount)
	    {
            if (!closeRecentFile(self))
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

    cacheInsert(fileInd);
}

int openFile(
    void*      self,
	char*      name, 
	int        flags, 
	int        mode)
{
	IFileManager _  = (IFileManager)self;

	CYCLE
	{
		int  fd = openFileBase(name, flags, mode);

	    if (fd >= 0)
		    return fd;
        
        if (!FILE_EMPTY_OR_AT_THE_END)
			return FILE_INVALID;

		if (_->closeRecentFile(_))
		    continue;

		return FILE_INVALID;
	}
	return FILE_INVALID;
}

void removeFileToFreeList(int fileInd)
{
    FCacheEl  fileItem = &fileCache[fileInd]; 

	if (fileItem->name != NULL)
	{
		free(fileItem->name);
		fileItem->name = NULL;
	}

	fileItem->state = 0;

	fileItem->nextFree    = fileCache[0].nextFree;
    fileCache[0].nextFree = fileInd;
}

int changeFileArrayPos(void* self, int fileId)
{
    if (fileCache[fileId].fileDesc == FILE_INVALID)
	{
        int retValue = insertIntoArray(self, fileId);
		if (retValue)
			return retValue;
		return 0;
	}

	if (fileCache[0].lessRecent != fileId)
	{
        removeFileToFreeList(fileId);
        insertIntoArray(self, fileId);
	}
	return 0;
}

long restoreFilePos(void* self, int fileId, long offset, int placeToPut)
{
	if (fileCache[fileId].fileDesc == FILE_INVALID)
	{
        if (placeToPut == SEEK_SET)
			fileCache[fileId].seekPos = offset;
		if (placeToPut == SEEK_CUR)
            fileCache[fileId].seekPos += offset;
		if (placeToPut == SEEK_END)
		{
            int retCode = changeFileArrayPos(self, fileId);
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

int openFileToCache(
    void*         self,
	char*         name, 
	int           flags, 
	int           mode)
{
	IFileManager   _       = (IFileManager)self;

    char*          nameCpy = strdup(name);
    int            ind     = _->cacheGetFree(_);
	FCacheEl       it      = &fileCache[ind];

	while (fileCount >= fileMaxCount)
	{
		if (!_->closeRecentFile(_))
		    break;
	}

	it->fileDesc = _->openFile(self, name, flags, mode);
	if (it->fileDesc < 0)
	{
		removeFileToFreeList(ind);
        free(nameCpy);
	    return FILE_INVALID;
	}

    fileCount++;
    _->cacheInsert(ind);

	it->name    = nameCpy;
	it->flags   = flags & ~(_O_CREAT | _O_TRUNC | _O_EXCL); 
	it->mode    = mode;
	it->seekPos = 0;
	it->size    = 0;
	it->state   = 0;

	return ind;
} 

#include "filemanager.h"
#include <stdio.h>

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
	openFile,
	estimateFileCount,
	reopenFile,
	deleteFileFromCache,
	writeFile
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

/* Calculates a number of file handlers already opened
 * and a number we can open without an exception.
 * Suppose that in our process there have been opened 
 * n file handlers. Its file descriptors will be numerated:
 * 1, 2, 3, 4,..., n - 1.
 * The others from n - 1 to max are not opened. Number of successful
 * dups is the number of not opened handlers.
 * To determine the max possible file dscriptor we know exactly that it
 * is not among already opened. During the dups we can find the max fd.
 */
void estimateFileCount(
	void*     self,
	int       max, 
	int*      maxToOpen, 
	int*      opened)
{   
    IFileManager    _      = (IFileManager)self;
    IMemoryManager  memMan = _->memManager;

	int*            fds;
	int             i;
	int             fdCount = 0;
	int             maxfd   = 0;
    int             size    = 1024;

	fds = (int*)memMan->alloc(size * sizeof(int));

    CYCLE
	{
		int   dupRes;

		/* Standard dup function from c library calls the 
		 * standard windows (or analoguos function in Linux) DuplicateHandle
		 * These handlers are duplicated inside the current process and 
		 * do not affect other processes */
		dupRes = dup(0);
		if (dupRes < 0)
			break;

        if (fdCount >= size)
		{
			size *= 2;
			fds = (int*)memMan->realloc(fds, size * sizeof(int));
		}

        fds[fdCount++] = dupRes;

		if (maxfd < dupRes)
			maxfd = dupRes;

		if (fdCount >= max)
			break;
	}

	for (i = 0; i < fdCount; i++)
		close(fds[i]);
    
	memMan->free(fds);

	*maxToOpen = fdCount;
	*opened    = maxfd + 1 - fdCount;
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

/* This function opens a file handler.
 * This function is an auxiliary for openFileToCache.
 * It is not called directly. 
 * openFileToCache function opens a file handler and puts it 
 * into the file ring. 
 * In one moment we can not open a file handler
 * and we get an error: Cannot create a file when that file already exists.
 * This means that we have already opened all allowed file handlers.
 * So that we need to close at least one file handler to allow 
 * opening a new one.
 * We choose the most recent file, remove it from the ring and close.
 */ 
int openFile(
    void*      self,
	char*      name, 
	int        flags, 
	int        mode)
{
	IFileManager _  = (IFileManager)self;

	CYCLE
	{
		int   fd     = openFileBase(name, flags, mode);

	    if (fd >= 0)
		    return fd;
        
		if (errno == FILE_PATH_NOT_FOUND)
			return FILE_INVALID;

		if (errno == EMFILE || errno == ENFILE)
		{
		    if (_->closeRecentFile(_))
		       continue;
		}

		return FILE_INVALID;
	}
}

/* The function adds a record to the head 
 * of the free list. (&fileCache[0])->nextFree is 
 * the head of free list. Also we free the name.
 */
void addToFreeList(void* self, int ind)
{
    IFileManager     _        = (IFileManager)self;
    IMemoryManager   memMan   = _->memManager;

    FCacheEl  theHead = &fileCache[0]; 
    FCacheEl  it      = &fileCache[ind]; 

	if (it->name != NULL)
	{
		memMan->free(it->name);
		it->name = NULL;
	}

	it->state = 0;

	it->nextFree      = theHead->nextFree;
    theHead->nextFree = ind;
}

/* Consider a situation: The system opens a file
 * for the first time and puts it into the ring.
 * Then we do not use the file for a specific period of time
 * and we want to use the file again. There can be
 * two different situations:
 *  1. File has not been removed from the ring yet
 *     and so has not been closed yet.
 *     In this case we move it to the head of 
 *     the less recent files.
 *   
 *  2. The file has been removed from the ring and closed.
 *     In this case we need to open it, seek to the 
 *     previous pointer position and insert into the ring
 */
int reopenFile(void* self, int ind)
{
    IFileManager   _           = (IFileManager)self;
    FCacheEl       theHead     = &fileCache[0];
    FCacheEl       it          = &fileCache[ind];
	int*           fd          = &(it->fileDesc);

	/* If the file is already opened we just  
	 * touch it and thats why it goes to the 
	 * least recent items.
	 */
	if FILE_IS_OPEN(it)
	{
		if (theHead->lessRecent != ind)
		{
			/* The next 2 code items make ind 
			 * as the least recent item and put 
			 * it into the head of the ring.
			 * To implement that we need first of all
			 * delete it from the ring and then 
			 * insert it again. cacheInsert function 
			 * inserts it exactly into the head.
			 */
			_->cacheDelete(ind);
			_->cacheInsert(ind);
		}
		return FM_SUCCESS;
	}

	/* Here the file is closed. So we need to open it 
	 * and put it to the head of less recent files.
	 * First of all we need to close some recent files
	 * if we have exceeded the files max count.
	 */
    while (fileCount >= fileMaxCount)
    {
		if (!_->closeRecentFile(_))
		    break;
    }

	/* Next we open a file handler */
	*fd = _->openFile(_, it->name, _O_EXCL, it->mode);
    if (*fd < 0)
        return *fd;
    
    fileCount++; 

	/* Move the file handler to the previous saved position. */
	if (it->seekPos != 0)
		lseek(*fd, it->seekPos, SEEK_SET);
    
    /* Insert into less recent files array. */
    _->cacheInsert(ind);

	return FM_SUCCESS;
}

/* This function restores the previous file content position */
long restoreFilePos(
	void*     self, 
	int       ind, 
	long      off, 
	int       placeToPut)
{
	IFileManager   _      = (IFileManager)self;
	int            pp     = placeToPut;
    FCacheEl       it     = &fileCache[ind];
	int            fd     = it->fileDesc;
	long*          spos   = &(it->seekPos);

	if (fd == FILE_INVALID)
	{
        if (pp == SEEK_SET)
			*spos = off;

		if (pp == SEEK_CUR)
			*spos += off;

		if (pp == SEEK_END)
		{
			int code = _->reopenFile(_, ind);
            if (code < 0)  
		        return code;
             
			fd    = (&fileCache[ind])->fileDesc;
			*spos = lseek(fd, off, pp);
		}
		return *spos;
	}

	if (pp == SEEK_SET && *spos != off)
		*spos = lseek(fd, off, pp);

	if (pp == SEEK_CUR && (off != 0 || *spos == -1))
        *spos = lseek(fd, off, pp);

	if (pp == SEEK_END)
		*spos = lseek(fd, off, pp);

	return *spos;
}

/* The function deletes the file from 
 * the cache, saves the current seek position
 * and closes it.
 */
void deleteFileFromCache(
    void*     self, 
    int       ind)
{
    IFileManager   _      = (IFileManager)self;
    FCacheEl       it     = &fileCache[ind];
	int            fd     = it->fileDesc;

	_->cacheDelete(ind);
    
	it->seekPos = lseek(fd, 0, SEEK_CUR);
	close(fd);

	fileCacheCount--;
	it->fileDesc = FILE_CLOSED;
}

/* The function opens a file and adds a file handler 
 * into the ring.
 */
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

	/* We maintain the maximum count of opened files.
	 * If we exceed the max count, we should close some 
	 * recent files to reduce the file number.
	 */
	while (fileCount >= fileMaxCount)
	{
		if (!_->closeRecentFile(_))
		    break;
	}

	it->fileDesc = _->openFile(self, name, flags, mode);

	/* If we have not opened a file we should return 
	 * the free index back to the free list. 
	 */
	if (it->fileDesc < 0)
	{
		addToFreeList(_, ind);
        free(nameCpy);
	    return FILE_INVALID;
	}

    fileCount++;
    _->cacheInsert(ind);

	it->name    = nameCpy;
	it->mode    = mode;
	it->seekPos = 0;
	it->size    = 0;
	it->state   = 0;

	return ind;
} 

int readFile(
    void*       self, 
    int         ind,
    char*       buf,
	int         len
)
{
    IFileManager   _       = (IFileManager)self;
    int            code    = _->reopenFile(_, ind);
    FCacheEl       it      = &fileCache[ind];
    DWORD		   error;

	if (code < 0)
		return code;

    CYCLE
	{
        code = read(it->fileDesc, buf, len);

		if (code >= 0)
		{
		    it->seekPos += code;
			break;
		}

		/*
		 * Windows may run out of kernel buffers and return "Insufficient
		 * system resources" error.  Wait a bit and retry to solve it.
		 *
		 * It is rumored that EINTR is also possible on some Unix filesystems,
		 * in which case immediate retry is indicated.
		 */
		error = GetLastError();
		if (error == ERROR_NO_SYSTEM_RESOURCES)
			continue;

		/* Trouble, so assume we don't know the file position anymore */
		it->seekPos = FILE_POS_INVALID;
		break;
	}
	return code;
}

/* The function writes a buffer into a file. */
void writeFile(
    void*       self, 
	int         ind,
	char*       buf,
	int         len)
{
    IFileManager   _       = (IFileManager)self;
    int            code    = _->reopenFile(_, ind);
	FCacheEl       it      = &fileCache[ind];
    DWORD		   error;

	if (code < 0)
		return code;
    
    CYCLE
	{
	    errno = 0;
	    code = write(it->fileDesc, buf, len);

	    /* if write didn't set errno, assume problem is no disk space */
	    if (code != len && errno == 0)
		    errno = ENOSPC;

	    if (code >= 0)
		{
		    it->seekPos += code;
			break;
		}

		/*
		 * Windows may run out of kernel buffers and return "Insufficient
		 * system resources" error.  Wait a bit and retry to solve it.
		 *
		 * It is rumored that EINTR is also possible on some Unix filesystems,
		 * in which case immediate retry is indicated.
		 */
		error = GetLastError();
		if (error == ERROR_NO_SYSTEM_RESOURCES)
			continue;

		/* Trouble, so assume we don't know the file position anymore */
		it->seekPos = FILE_POS_INVALID;
		break;
	}
	return code;
}

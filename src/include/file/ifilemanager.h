
#ifndef IFILE_MANAGER_H
#define IFILE_MANAGER_H

#include "imemorymanager.h"
#include "ierrorlogger.h"

typedef struct SIFileManager
{
	IMemoryManager             memManager;
    IErrorLogger               errorLogger; 

	void (*ctorFileMan)        (void* self);
	int  (*openFileToCache)    (void* self, char* name, int flags, int mode);
	long (*restoreFilePos)     (void* self, int fileId, long offset, int placeToPut);
	void (*cacheInsert)        (int pos);
	void (*cacheRealloc)       (void* self);
    int  (*cacheGetFree)       (void* self);
	void (*cacheDelete)        (int pos);
	Bool (*closeRecentFile)    (void* self);
	int  (*openFile)           (void* self, char* name, int flags, int mode);
	void (*estimateFileCount)  (void* self, int max, int* maxToOpen, int* opened);
	int  (*reopenFile)         (void* self, int ind);
	void (*deleteFileFromCache)(void* self, int ind);
	void (*writeFile)          (void* self, int ind, char* buf, int len);
	int  (*readFile)           (void* self, int ind, char* buf, int len);
} SIFileManager, *IFileManager;


#endif
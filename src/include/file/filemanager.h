#include "ifilemanager.h"
#include "common.h"
#include "relfile.h"
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <errno.h>
#include "osfile.h"
#include "file.h"
#include "trackmemmanager.h"

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

extern const SIFileManager sFileManager;
extern const IFileManager  fileManager;

extern int	     allocatedDescsCount;
extern int		 maxFileDescriptors;

extern FCacheEl         fileCache;
extern size_t           fileCacheCount;
extern int	            fileCount;

#define FILE_INVALID          (-1)
#define MIN_CACHE_EL_TO_ALLOC 32
#define FILE_EMPTY_OR_AT_THE_END ((errno) == EMFILE || (errno) == ENFILE)

void ctorFileMan     (void* self);
int  openFileToCache (void* self, char* name, int flags, int mode);
long restoreFilePos  (void* self, int fileId, long offset, int placeToPut);
void cacheInsert     (int pos);
void cacheRealloc    (void* self);
int  cacheGetFree    (void* self);
void cacheDelete     (int pos);
Bool closeRecentFile (void* self);
int openFile         (void* self, char* name, int flags, int mode);


#endif


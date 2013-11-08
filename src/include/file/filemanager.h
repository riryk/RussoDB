#include "ifilemanager.h"
#include "common.h"
#include "relfile.h"
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <errno.h>
#include "osfile.h"

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

extern const SIFileManager sFileManager;
extern const IFileManager  fileManager;

typedef struct SFileCacheItem
{
	int			   fileDesc;			
	uint16         state;		
	int		       nextFree;
	int		       moreRecent;	
	int		       lessRecent;
	long		   seekPos;
	long		   size;
	char*          name;
	int			   flags;
	int			   mode;
} SFileCacheItem, *FileCacheItem;

FileCacheItem    fileCache;
size_t           sizeFileCache = 0;
int	             fileCount     = 0;

int	             allocatedDescsCount = 0;
int			     maxFileDescriptors  = 32;

#define FILE_CLOSED (-1)

int openFile(char* fname, int fileFlags, int fileMode);
long restoreFilePos(int fileId, long offset, int placeToPut);

#endif
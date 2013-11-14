
#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <io.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "error.h"
#include "common.h"
#include "osfile.h"

typedef unsigned long long int uint64;

typedef struct SFCacheEl
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
} SFCacheEl, *FCacheEl;

void Init();
void DetermineMaxAllowedFileDescriptors();
int ROpenFile(char* FileName, int FileFlags, int FileMode);
int OpenTempFileInTablespace(int tableSpaceId, int error);

#endif

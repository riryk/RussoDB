
#ifndef IREL_FILE_MANAGER_H
#define IREL_FILE_MANAGER_H

#include "ifilemanager.h"
#include "imemorymanager.h"
#include "relfile.h"

typedef struct SIRelFileManager
{
	IFileManager   fileManager;
	IMemoryManager memManager;
	int (*getBlocksNum)(
          void*           self,
	      RelData         rel, 
	      FilePartNumber  partnum);
} SIRelFileManager, *IRelFileManager;


#endif
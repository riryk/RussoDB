
#ifndef IREL_FILE_MANAGER_H
#define IREL_FILE_MANAGER_H

#include "ifilemanager.h"
#include "imemorymanager.h"
#include "relfile.h"

typedef struct SIRelFileManager
{
	IFileManager   fileManager;
	IMemoryManager memManager;

	void  (*ctorRelFileMan) (void* self);
    
	void  (*createRelPart)(
          void*           self,
	      RelData         rel, 
	      int             pnum);

	char* (*getFilePath)(
          void*           self,
	      RelFileInfo     relFile, 
	      int             backend, 
	      int             part);

	int (*getBlocksNum)(
          void*           self,
	      RelData         rel, 
	      FilePartNumber  partnum);

	FileSeg (*openRel)(
          void*           self,
	      RelData         rel, 
	      FilePartNumber  pnum);
} SIRelFileManager, *IRelFileManager;


#endif
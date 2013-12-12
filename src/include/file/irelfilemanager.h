
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
    
	FileSeg  (*createRelPart)(
          void*           self,
		  char*           execfold,
	      RelData         rel, 
	      int             pnum);

	char* (*getFilePath)(
          void*           self,
		  char*           execfold,
	      RelFileInfo     relFile, 
	      int             backend, 
	      int             part);

	int (*getBlocksNum)(
          void*           self,
          char*           fold,
	      RelData         rel, 
	      FilePartNumber  part,
		  int             segmsize);

	FileSeg (*openRel)(
          void*           self,
		  char*           execfold,
	      RelData         rel, 
	      FilePartNumber  pnum);

	FileSeg (*openRelSegm)(
          void*           self,
	      char*           execfold,
	      RelData         rel, 
	      FilePartNumber  part,
	      uint            segnum,
	      int             flags);

	void (*closeSegm)(
          void*           self,
          FileSeg         seg);

	FileSeg (*findBlockSegm)(
          void*               self,
	      char*               fold,
	      RelData             rel, 	
	      FilePartNumber      part,
	      uint                block,
	      ExtensionBehavior   behavior,
		  int                 segmsize);

	FileSeg (*writeBlock)(
          void*            self,
          char*            fold,
	      RelData          rel, 	
	      FilePartNumber   part,
          uint             block,
	      char*            buffer);

	void (*pushFSyncRequest)(
          void*           self,
          char*           fold,
	      RelData         rel, 
	      FilePartNumber  part,
 	      FileSeg         seg);

} SIRelFileManager, *IRelFileManager;


#endif
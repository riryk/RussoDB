#include "irelfilemanager.h"
#include "trackmemmanager.h"
#include "common.h"
#include "relfile.h"
#include "filemanager.h"
#include "snprintf.h"
#include "hashtable.h"
#include <fcntl.h>

#ifndef REL_FILE_MANAGER_H
#define REL_FILE_MANAGER_H

#define TBL_SPACE_GLOBAL "global"
#define TBL_SPACE_DEFAULT "default"
#define TBL_SPACES "tblspaces"
#define REL_PART_LEN 4
#define REL_SEGM_LEN 12


extern const SIRelFileManager sRelFileManager;
extern const IRelFileManager  relFileManager;


void ctorRelFileMan(void* self);

FileSeg createRelPart(
    void*            self,
	char*            execfold,
	RelData          rel, 
	int              pnum);

int getBlocksNum(
    void*            self,
    char*            fold,
	RelData          rel, 
	FilePartNumber   part,
	int              segmsize);

char* getFilePath(
    void*            self,
	char*            execfold,
	RelFileInfo      relFile, 
	int              backend, 
	int              part);

FileSeg openRel(
    void*            self,
	char*            execfold,
	RelData          rel, 
	FilePartNumber   pnum);

FileSeg openRelSegm(
    void*            self,
	char*            execfold,
	RelData          rel, 
	FilePartNumber   part,
	uint             segnum,
	int              flags);

void closeSegm(
    void*            self,
    FileSeg          seg);

FileSeg findBlockSegm(
    void*               self,
	char*               fold,
	RelData             rel, 	
	FilePartNumber      part,
	uint                block,
	ExtensionBehavior   behavior,
	int                 segmsize);

FileSeg writeBlock(
    void*            self,
    char*            fold,
	RelData          rel, 	
	FilePartNumber   part,
    uint             block,
	char*            buffer);

void pushFSyncRequest(
    void*            self,
    char*            fold,
	RelData          rel, 
	FilePartNumber   part,
	FileSeg          seg);

#endif
#include "irelfilemanager.h"
#include "trackmemmanager.h"
#include "common.h"
#include "relfile.h"
#include "filemanager.h"
#include "snprintf.h"
#include <fcntl.h>

#ifndef REL_FILE_MANAGER_H
#define REL_FILE_MANAGER_H

#define TBL_SPACE_GLOBAL "global"
#define TBL_SPACE_DEFAULT "default"
#define TBL_SPACES "tblspaces"
#define REL_PART_LEN 4

void ctorRelFileMan(void* self);

void createRelPart(
    void*         self,
	RelData       rel, 
	int           pnum);

int getBlocksNum(
    void*            self,
	RelData          rel, 
	FilePartNumber   partnum);

char* getFilePath(
    void*            self,
	RelFileInfo      relFile, 
	int              backend, 
	int              part);

FileSeg openRel(
    void*            self,
	RelData          rel, 
	FilePartNumber   pnum);

#endif
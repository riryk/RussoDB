#include "irelfilemanager.h"
#include "trackmemmanager.h"
#include "common.h"
#include "relfile.h"
#include "filemanager.h"
#include <fcntl.h>

#ifndef REL_FILE_MANAGER_H
#define REL_FILE_MANAGER_H

int getBlocksNum(
    void*            self,
	RelData          rel, 
	FilePartNumber   partnum);

#endif
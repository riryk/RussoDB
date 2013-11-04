#include "iRelFileManager.h"
#include "common.h"
#include "relfile.h"

#ifndef REL_FILE_MANAGER_H
#define REL_FILE_MANAGER_H

typedef struct SFileSeg
{
	int		          fileDesc;
	uint              segNum;
	struct SFileSeg*  segNext
} SFileSeg, *FileSeg;

#endif
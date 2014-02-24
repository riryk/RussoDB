
#ifndef ICOMMON_H
#define ICOMMON_H

#include <stdlib.h>
#include <string.h>

typedef struct SICommon
{
	int   (*nextPowerOf2)(long num);
	void  (*setExecFold)(char* fold);
	char* (*getExecFold)(); 
} SICommon, *ICommon;

#endif
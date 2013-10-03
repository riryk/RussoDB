
#include <stdlib.h>
#include <string.h>

#ifndef ICOMMON_H
#define ICOMMON_H

typedef struct SICommon
{
	int (*nextPowerOf2)(long num);
} SICommon, *ICommon;

#endif
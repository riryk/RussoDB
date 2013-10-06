#include "common.h"

#ifndef IHASHTABLE_HELPER_H
#define IHASHTABLE_HELPER_H

typedef struct SIHashtableHelper
{
	int  (*calcSegmsNum)(int numHashLists, ulong segmSize);
	uint (*calcLowMask)(int numHashLists);
	uint (*calcHighMask)(int numHashLists);
} SIHashtableHelper, *IHashtableHelper;

#endif
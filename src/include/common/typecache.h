
#ifndef TypeCache_h
#define TypeCache_h

#include "common.h"

typedef struct TypeCacheEntry
{
	int16 typeLength;
	Bool typeByValue;
	char typeAlign;
	void* comparisonFunction;
	struct TypeCacheEntry* rangeElementType;
} TypeCacheEntry;

TypeCacheEntry* lookupTypeCache(uint typeId);

#endif
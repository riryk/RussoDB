#ifndef Rel_scan_h
#define Rel_scan_h

#include "common.h"
#include "rel.h"

typedef struct ScanKeyData
{
	uint attributeNumber;
} ScanKeyData;

typedef ScanKeyData* ScanKey;

typedef struct IndexScanDescriptorData
{
	Relation heapRelation;
	Relation indexRelation;	
	int numberOfKeys;
	ScanKey keyData;
	void* opaque;
} IndexScanDescriptorData;

#endif






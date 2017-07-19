#ifndef Rel_scan_h
#define Rel_scan_h

#include "common.h"
#include "rel.h"

typedef struct IndexScanDescriptorData
{
	Relation heapRelation;
	Relation indexRelation;	
	void* opaque;
} IndexScanDescriptorData;

#endif






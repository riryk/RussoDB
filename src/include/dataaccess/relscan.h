#ifndef Rel_scan_h
#define Rel_scan_h

#include "common.h"
#include "rel.h"
#include "datum.h"

typedef struct ScanKeyData
{
	uint attributeNumber;
	uint strategy;
	int flags;
    Datum argument;
} ScanKeyData;

typedef ScanKeyData* ScanKey;

#define ScanKeyIsNull 0x0001
#define ScanKeySearchForNull 0x0040 /* scankey represents "col IS NULL" */
#define ScanKeySearchForNotNull 0x0080 /* scankey represents "col IS NOT NULL" */ 

typedef struct IndexScanDescriptorData
{
	Relation heapRelation;
	Relation indexRelation;	
	int numberOfKeys;
	ScanKey keyData;
	void* opaque;
} IndexScanDescriptorData;

#endif






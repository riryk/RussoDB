#ifndef Index_h
#define Index_h

#include "relscan.h"

typedef struct IndexScanDescriptorData* IndexScanDescriptor;

IndexScanDescriptor RelationGetIndexScan(Relation indexRelation, int numberOfKeys, int numberOfOrderBys);

#endif






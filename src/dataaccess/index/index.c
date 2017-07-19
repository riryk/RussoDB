#include "index.h"

IndexScanDescriptor RelationGetIndexScan(Relation indexRelation, int numberOfKeys, int numberOfOrderBys)
{
    IndexScanDescriptor scanDescriptor = (IndexScanDescriptor)malloc(sizeof(IndexScanDescriptorData));

    scanDescriptor->heapRelation = NULL;
	scanDescriptor->indexRelation = indexRelation;

	return scanDescriptor;
}






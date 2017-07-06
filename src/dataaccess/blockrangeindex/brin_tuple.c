#include "brin_tuple.h"

BrinMemoryTuple* brinNewMemoryTuple(BrinDescriptor* brinDescriptor);

BrinMemoryTuple* brinDeformTuple(BrinDescriptor* brinDescriptor, BrinTuple* brinTuple)
{
	BrinMemoryTuple* brinMemoryTuple = brinNewMemoryTuple(brinDescriptor);

	Datum* values = malloc(sizeof(Datum) * brinDescriptor->totalStored);

	Bool* allNulls = malloc(sizeof(Bool) * brinDescriptor->tupleDescriptor->attributesNumber);

    Bool* hasNulls = malloc(sizeof(Bool) * brinDescriptor->tupleDescriptor->attributesNumber);

    return NULL;   
}

BrinMemoryTuple* brinNewMemoryTuple(BrinDescriptor* brinDescriptor)
{
	int tupleAttributesSize = sizeof(BrinTupleValues) * brinDescriptor->tupleDescriptor->attributesNumber;

    int tupleBaseSize = sizeof(BrinMemoryTuple) + tupleAttributesSize;
    
    BrinMemoryTuple* brinTuple = malloc(tupleBaseSize + sizeof(Datum) * brinDescriptor->totalStored);

    char* datum = (char*)brinTuple + tupleBaseSize;

    int attrIndex;

    for (attrIndex = 0; attrIndex < brinDescriptor->tupleDescriptor->attributesNumber; attrIndex++)
	{
        brinTuple->columnValues[attrIndex].attributeNumber = attrIndex + 1;
        brinTuple->columnValues[attrIndex].allNulls = True;
        brinTuple->columnValues[attrIndex].hasNulls = False;
        brinTuple->columnValues[attrIndex].values = (Datum*)datum;

        datum += sizeof(Datum) * brinDescriptor->columns[attrIndex]->numberOfColumnsStored;
	}   

	return brinTuple;
}
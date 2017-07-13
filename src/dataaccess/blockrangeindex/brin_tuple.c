#include "brin_tuple.h"

typedef struct BrinTupleData
{
    Datum* values;
    Bool* allNulls;
    Bool* hasNulls;
} BrinTupleData;

BrinMemoryTuple* brinNewMemoryTuple(BrinDescriptor* brinDescriptor);

BrinTupleData* brinDeconstructTuple(BrinDescriptor* brinDescriptor, BrinTuple* brinTuple);

BrinTupleData* brinNewTupleData(BrinDescriptor* brinDescriptor);

TupleDescriptor brinGetTupleDescriptor(BrinDescriptor* brinDescriptor);

BrinMemoryTuple* brinDeformTuple(BrinDescriptor* brinDescriptor, BrinTuple* brinTuple)
{
	BrinMemoryTuple* brinMemoryTuple = brinNewMemoryTuple(brinDescriptor);

    BrinTupleData* tupleData = brinDeconstructTuple(brinDescriptor, brinTuple); 

    free(tupleData->values);

    free(tupleData->allNulls);

    free(tupleData->hasNulls);

    free(tupleData);    

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

BrinTupleData* brinDeconstructTuple(BrinDescriptor* brinDescriptor, BrinTuple* brinTuple)
{
    BrinTupleData* tupleData = brinNewTupleData(brinDescriptor);
    
    char* tupleDataPointer = (char*)brinTuple + BrinTupleDataOffset(brinTuple);

    Bool tupleHasNulls = BrinTupleHasNulls(brinTuple);

    uint8* nullBitPointer = (tupleHasNulls) 
                          ? (uint8*)((char*)brinTuple + SizeOfBrinTuple)  
                          : NULL;

    int attributesNumber = brinDescriptor->tupleDescriptor->attributesNumber;

    int attrIndex;

    for (attrIndex = 0; attrIndex < attributesNumber; attrIndex++)
    {
        tupleData->allNulls[attrIndex] = tupleHasNulls && !AttributeIsNull(attrIndex, nullBitPointer);
    
        tupleData->hasNulls[attrIndex] = tupleHasNulls && !AttributeIsNull(attributesNumber + attrIndex, nullBitPointer);           
    }   

    return tupleData;
}

BrinTupleData* brinNewTupleData(BrinDescriptor* brinDescriptor)
{
    BrinTupleData* tupleData = malloc(sizeof(BrinTupleData));    

    tupleData->values = malloc(sizeof(Datum) * brinDescriptor->totalStored);

    tupleData->allNulls = malloc(sizeof(Bool) * brinDescriptor->tupleDescriptor->attributesNumber);

    tupleData->hasNulls = malloc(sizeof(Bool) * brinDescriptor->tupleDescriptor->attributesNumber);

    return tupleData;
}

void extractTupleValues(BrinTupleData* tupleData, BrinDescriptor* brinDescriptor)
{
    TupleDescriptor tupleDescriptor = brinGetTupleDescriptor(brinDescriptor);

    int storedDataOffset = 0;

    long offset = 0;

    int attrIndex, columnIndex;

    for (attrIndex = 0; attrIndex < brinDescriptor->tupleDescriptor->attributesNumber; attrIndex++)
    {
        int numberOfColumns = brinDescriptor->columns[attrIndex]->numberOfColumnsStored;

        if (tupleData->allNulls[attrIndex])
        {
            storedDataOffset += numberOfColumns;

            continue;
        }
         
        for (columnIndex = 0; columnIndex < numberOfColumns; columnIndex++)
        {
            //RelAttribute attribute = tupleDescriptor->attributes[columnIndex];

            //tupleDescriptor->attributesNumber
        }   
    }    
}

TupleDescriptor brinGetTupleDescriptor(BrinDescriptor* brinDescriptor)
{
    return brinDescriptor->tupleDescriptor;
}
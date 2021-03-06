#include "brin_tuple.h"
#include "datum.h"

typedef struct BrinTupleData
{
    Datum* values;
    Bool* allNulls;
    Bool* hasNulls;
} BrinTupleData;

BrinMemoryTuple* brinNewMemoryTuple(BrinDescriptor* brinDescriptor);

BrinTupleData* brinDeconstructTuple(BrinDescriptor* brinDescriptor, BrinTuple* brinTuple);

BrinTupleData* brinNewTupleData(BrinDescriptor* brinDescriptor);

void extractTupleValues(BrinTupleData* tupleData, BrinDescriptor* brinDescriptor, char* tupleDataPointer);

TupleDescriptor brinGetTupleDescriptor(BrinDescriptor* brinDescriptor);

BrinTuple* brinCopyTuple(BrinTuple*tuple, size_t length)
{
    BrinTuple* newTuple = malloc(length);

    memcpy(newTuple, tuple, length);

    return newTuple;
}

void brinMemoryTupleInitialize(BrinMemoryTuple* brinTuple, BrinDescriptor* brinDescriptor)
{
    int attrIndex;
    for (attrIndex = 0; attrIndex < brinDescriptor->tupleDescriptor->attributesNumber; attrIndex++)
    {
        brinTuple->columnValues[attrIndex].allNulls = True;
        brinTuple->columnValues[attrIndex].hasNulls = False;
    }
}

FormBrinTupleResult* brinFormTuple(BrinDescriptor* brinDescriptor, BlockNumber block, BrinMemoryTuple* tuple)
{
    // TODO: Convert memory tuple to on disk tuple
    return (FormBrinTupleResult*)malloc(sizeof(FormBrinTupleResult));
}

BrinTuple* brinFormPlaceholderTuple(BrinDescriptor* brinDescriptor, BlockNumber block, size_t* tupleSizeComputed)
{
    // TODO: Create an empty placeholder tuple
    return (BrinTuple*)malloc(sizeof(BrinTuple));
}

BrinMemoryTuple* brinDeformTuple(BrinDescriptor* brinDescriptor, BrinTuple* brinTuple)
{
	BrinMemoryTuple* brinMemoryTuple = brinNewMemoryTuple(brinDescriptor);

    BrinTupleData* tupleData = brinDeconstructTuple(brinDescriptor, brinTuple); 

    int dataValueIndex = 0;

    int attrIndex;

    for (attrIndex = 0; attrIndex < brinDescriptor->tupleDescriptor->attributesNumber; attrIndex++)
    {
        int columnValueIndex;

        BrinTupleValues brinMemoryTupleColumn = brinMemoryTuple->columnValues[attrIndex];
        
        BrinColumnDescriptor* column = brinDescriptor->columns[attrIndex];

        if (tupleData->allNulls[attrIndex])
        {
            dataValueIndex += column->totalStored;
            
            continue;
        }

        for (columnValueIndex = 0; columnValueIndex < column->totalStored; columnValueIndex++)
        {
            TypeCacheEntry* typeCacheItem = column->typeCache[columnValueIndex];

            brinMemoryTupleColumn.values[columnValueIndex] = DatumCopy(tupleData->values[dataValueIndex++], typeCacheItem->typeByValue, typeCacheItem->typeLength);
        }
    }

    free(tupleData->values);

    free(tupleData->allNulls);

    free(tupleData->hasNulls);

    free(tupleData);    

    return brinMemoryTuple;   
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

        datum += sizeof(Datum) * brinDescriptor->columns[attrIndex]->totalStored;
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

    extractTupleValues(tupleData, brinDescriptor, tupleDataPointer);

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

void extractTupleValues(BrinTupleData* tupleData, BrinDescriptor* brinDescriptor, char* tupleDataPointer)
{
    TupleDescriptor tupleDescriptor = brinGetTupleDescriptor(brinDescriptor);

    int stored = 0;

    long offset = 0;

    int attrIndex, columnIndex;

    for (attrIndex = 0; attrIndex < brinDescriptor->tupleDescriptor->attributesNumber; attrIndex++)
    {
        int numberOfColumns = brinDescriptor->columns[attrIndex]->totalStored;

        if (tupleData->allNulls[attrIndex])
        {
            stored += numberOfColumns;

            continue;
        }
         
        for (columnIndex = 0; columnIndex < numberOfColumns; columnIndex++)
        {
            RelAttribute attribute = tupleDescriptor->attributes[columnIndex];

            offset = AttributeAlignNominal(offset);

            tupleData->values[stored++] = FetchAttribute(attribute, tupleDataPointer);

            offset = AttributeAddLengthPointer(offset, attribute->len, tupleDataPointer + offset);
        }   
    }    
}

TupleDescriptor brinGetTupleDescriptor(BrinDescriptor* brinDescriptor)
{
    return brinDescriptor->tupleDescriptor;
}
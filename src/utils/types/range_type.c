#include "range_type.h"
#include "datum.h"
#include "typecache.h"
#include "tuple.h"

typedef struct RangeDeserializeResult
{
	RangeBound leftBound;
	RangeBound rightBound;
	Bool isEmpty;
} RangeDeserializeResult;

char rangeGetFlags(RangeType* range);

TypeCacheEntry* rangeGetTypeCache(uint rangeTypeId);

RangeDeserializeResult* rangeDeserialize(TypeCacheEntry* typeCacheItem, RangeType* range);

RangeDeserializeResult* allocateRangeDeserializeResult();

Bool CheckRangeIsEmpty(Datum datum)
{
	RangeType* range = (RangeType*)datum;

	char rangeFlags = rangeGetFlags(range);

	return rangeFlags & RangeEmpty;
}

char rangeGetFlags(RangeType* range)
{
	return *((char*)range + VarSize(range) - 1);
}

Datum RangeMergeElement(Datum rangeDatum, Datum element)
{
	//TODO: Merge element with range. If it is lower than lower bound, change lower bound. 
	// If it is bigger than upper bound, change upper bound
	return NULL;
}

Bool RangeBeforeElement(Datum rangeDatum, Datum element)
{
    //TODO: Check that range is before element, 
    // which is the same as to check that element is bigger than upper bound
    return False;
}

Bool RangeContainsElement(Datum rangeDatum, Datum element)
{
	RangeType* range = RangeTypeConvertFromDatum(rangeDatum);

	TypeCacheEntry* rangeTypeCacheItem = rangeGetTypeCache(RangeTypeGetOid(range));

    Bool rangeContainsElement = rangeContainsElementInternal(rangeTypeCacheItem, range, element);     

	return rangeContainsElement;
}

Bool rangeContainsElementInternal(TypeCacheEntry* typeCacheItem, RangeType* range, Datum value)
{
    RangeDeserializeResult* rangeResult = rangeDeserialize(typeCacheItem, range);
    CompareRangeBoundValue compareRangeBound = (CompareRangeBoundValue)typeCacheItem->comparisonFunction;
    Bool rangeContainsElement = True;
    
    if (!rangeResult->leftBound.infinite)
    {
        int leftBoundComparisonResult = compareRangeBound(rangeResult->leftBound.value, value);
        Bool newValueIsLowerThanLowerBound = leftBoundComparisonResult < 0;
        Bool newValueIsEqualToLowerBound = leftBoundComparisonResult == 0;

        rangeContainsElement = newValueIsLowerThanLowerBound 
                           || (newValueIsEqualToLowerBound && !rangeResult->leftBound.inclusive);
    }
    else if (!rangeResult->rightBound.infinite)
	{
	    int rightBoundComparisonResult = compareRangeBound(rangeResult->rightBound.value, value); 
	    Bool newValueIsBiggerThanUpperBound = rightBoundComparisonResult < 0;
        Bool newValueIsEqualToUpperBound = rightBoundComparisonResult == 0;

        rangeContainsElement = newValueIsBiggerThanUpperBound 
                           || (newValueIsEqualToUpperBound && !rangeResult->rightBound.inclusive);
	}
	
    free(rangeResult);

    return rangeContainsElement;
}

Bool rangeContainsInternal(TypeCacheEntry* typeCacheItem, RangeType* range1, RangeType* range2)
{
    RangeDeserializeResult* rangeResult1 = rangeDeserialize(typeCacheItem, range1);
	RangeDeserializeResult* rangeResult2 = rangeDeserialize(typeCacheItem, range2);

    Bool lowerBound1IsLessThanLowerBound2 = rangeCompareBounds(typeCacheItem, rangeResult1->leftBound, rangeResult2->leftBound) > 0;
    Bool upperBound1IsMoreThanUpperBound2 = rangeCompareBounds(typeCacheItem, rangeResult1->rightBound, rangeResult2->rightBound) < 0;

    Bool rangeContainsElement = rangeResult2->isEmpty 
                           || (!rangeResult1->isEmpty && 
                           	   lowerBound1IsLessThanLowerBound2 && upperBound1IsMoreThanUpperBound2);

    free(rangeResult1);
    free(rangeResult2);

    return rangeContainsElement;
}

int rangeCompareBounds(TypeCacheEntry* typCacheItem, RangeBound* bound1, RangeBound* bound2)
{
	// TODO: Compare two range boundary points, returning <0, 0, or >0 
	//       according to whether bound1 is less than, equal to, or greater than bound2.
    return 0;
}

TypeCacheEntry* rangeGetTypeCache(uint rangeTypeId)
{
	TypeCacheEntry* rangeTypeCacheItem = lookupTypeCache(rangeTypeId);
	
	return rangeTypeCacheItem;
}

RangeDeserializeResult* rangeDeserialize(TypeCacheEntry* typeCacheItem, RangeType* range)
{
    RangeDeserializeResult* result = allocateRangeDeserializeResult();

    char rangeFlags = rangeGetFlags(range);

    TypeCacheEntry* rangeElementType = typeCacheItem->rangeElementType;
    
    char* rangeBoundsPointer = (char*)(range + 1);

    if (RangeHasLeftBound(rangeFlags))
	{
		result->leftBound.value = FetchAttributeBase(rangeBoundsPointer, rangeElementType->typeByValue, rangeElementType->typeLength);
		result->leftBound.infinite = (rangeFlags & RangeLeftBoundIsInfinity) != 0;
		result->leftBound.inclusive = (rangeFlags & RangeLeftBoundIsInclusive) != 0;
		result->leftBound.lower = True;

		rangeBoundsPointer = AttributeAddLengthPointer(rangeBoundsPointer, rangeElementType->typeByValue, rangeBoundsPointer);
	}    

    if (RangeHasRightBound(rangeFlags))
    {
    	result->rightBound.value = FetchAttributeBase(rangeBoundsPointer, rangeElementType->typeByValue, rangeElementType->typeLength);
		result->rightBound.infinite = (rangeFlags & RangeRightBoundIsInfinity) != 0;
		result->rightBound.inclusive = (rangeFlags & RangeRightBoundIsInclusive) != 0;
		result->rightBound.lower = False;
    }

    return result;
}

RangeDeserializeResult* allocateRangeDeserializeResult()
{
	RangeDeserializeResult* result = malloc(sizeof(RangeDeserializeResult));

    result->leftBound.value = 0;
    result->rightBound.value = 0;

    return result;
}



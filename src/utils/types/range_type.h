#ifndef Range_type_h
#define Range_type_h

#include "common.h"
#include "datum.h"

#define RangeEmpty 0x01

typedef struct
{
	int length;
	uint rangetypeId;	
	/* Following the rangetypeId are zero to two bound values, then a flags byte */
} RangeType;

typedef struct
{
	Datum value;
	Bool infinite; /* bound is +/- infinity */
	Bool inclusive; /* bound is inclusive (vs exclusive) */
	Bool lower; /* this is the lower (vs upper) bound */
} RangeBound;

typedef int (*CompareRangeBoundValue)(Datum, Datum);

#define RangeIsEmpty 0x01

#define RangeLeftBoundIsInfinity 0x08
#define RangeLeftBoundIsNull 0x20
#define RangeLeftBoundIsInclusive 0x02

#define RangeRightBoundIsInfinity 0x10
#define RangeRightBoundIsNull 0x40
#define RangeRightBoundIsInclusive 0x04	

#define RangeHasLeftBound(rangeFlags) (!((rangeFlags) & (RangeIsEmpty | \
											             RangeLeftBoundIsNull | \
											             RangeLeftBoundIsInfinity)))

#define RangeHasRightBound(rangeFlags) (!((rangeFlags) & (RangeIsEmpty | \
											              RangeRightBoundIsNull | \
											              RangeRightBoundIsInfinity)))

#define RangeTypeGetOid(range) ((range)->rangetypeId)

#define RangeTypeConvertFromDatum(datum) ((RangeType*)datum)

Bool CheckRangeIsEmpty(Datum datum);

Bool RangeContainsElement(Datum rangeDatum, Datum element);

Datum RangeMergeElement(Datum rangeDatum, Datum element);

#endif
#include "brin_inclusion_strategy.h"
#include "datum.h"
#include "range_type.h"
#include "strategynumber.h"

Bool checkNullConsistensy(ScanKey scanKey);

Bool brinInclusionConsistent(BrinDescriptor* brinDescriptor, BrinTupleValues* column, ScanKey scanKey)
{
    if (scanKey->flags & ScanKeyIsNull)
    {
        return checkNullConsistensy(column, scanKey);
    }

    if (column->allNulls)
    {
    	return False;
    }

    switch (scanKey->strategy)
	{
        case LeftStrategyNumber:
            return overRightStrategy(column->values[InclusionUnion], scanKey->argument);
		default:
			return False;
	}

    return False;
}

Bool checkNullConsistensy(BrinTupleValues* column, ScanKey scanKey)
{
    if (scanKey->flags & ScanKeySearchForNull)
    {
        return column->allNulls || column->hasNulls;
    }

    if (scanKey->flags & ScanKeySearchForNotNull)
    {
    	return !column->allNulls;
    }

    return False;
}

Bool overRightStrategy(Datum brinRangeValue, Datum scanKeyElement)
{
	Bool rangeIsBeforeElement;

    if (CheckRangeIsEmpty(brinRangeValue))
    {
        return False;
    }
	
    rangeIsBeforeElement = RangeBeforeElement(brinRangeValue, scanKeyElement);
	
    return !rangeIsBeforeElement;
}
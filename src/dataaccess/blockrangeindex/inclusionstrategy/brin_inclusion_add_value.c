#include "brin_inclusion_strategy.h"
#include "datum.h"
#include "range_type.h"

void setAllNulls(BrinTupleValues* column, RelAttribute attribute, Datum newValue);

Bool brinInclusionAddValue(BrinDescriptor* brinDescriptor, BrinTupleValues* column, Datum newValue, Bool isNull)
{
	int attrNumber = column->attributeNumber;

	RelAttribute attribute = brinDescriptor->tupleDescriptor->attributes[attrNumber - 1];

	if (isNull)
	{
		return setHasNulls(column);
	}

    if (column->allNulls)
	{
		setAllNulls(column, attribute, newValue);
	}

    if (DatumGetBool(column->values[InclusionUnmergeable]))
    {
        return False;
    }
    
    if (CheckRangeIsEmpty(newValue))
    {
    	return setContainsEmpty(column);
    }
    
    if (RangeContainsElement(column->values[InclusionUnion], newValue))
    {
    	return False;
    }

    if (!attribute->byVal)
    {
    	free(column->values[InclusionUnion]);
    }

    {
        Datum newRange = RangeMergeElement(column->values[InclusionUnion], newValue);

        column->values[InclusionUnion] = newRange;
    }

	return True;
}

Bool setHasNulls(BrinTupleValues* column)
{
    if (column->hasNulls)
	{
		return False;
	}

	column->hasNulls = True;

	return True;
}

void setAllNulls(BrinTupleValues* column, RelAttribute attribute, Datum newValue)
{
	column->values[InclusionUnion] = DatumCopy(newValue, attribute->byVal, attribute->len);
    column->values[InclusionUnmergeable] = False;
	column->values[InclusionContainsEmpty] = False;
	column->allNulls = False;
}

Bool setContainsEmpty(BrinTupleValues* column)
{
	if (!DatumGetBool(column->values[InclusionContainsEmpty]))
	{
        column->values[InclusionContainsEmpty] = BoolGetDatum(True);

        return True;
	}

	return False;
}
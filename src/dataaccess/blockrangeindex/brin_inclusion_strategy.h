#include "common.h"
#include "brin_internal.h"
#include "brin_tuple.h"

Bool brinInclusionAddValue(BrinDescriptor* brinDescriptor, BrinTupleValues* column, Datum newValue, Bool isNull);
#include "common.h"
#include "brin_internal.h"
#include "brin_tuple.h"
#include "index.h"

#define InclusionUnion 0
#define InclusionUnmergeable 1
#define InclusionContainsEmpty 2

Bool brinInclusionAddValue(BrinDescriptor* brinDescriptor, BrinTupleValues* column, Datum newValue, Bool isNull);

Bool brinInclusionConsistent(BrinDescriptor* brinDescriptor, BrinTupleValues* column, ScanKey scanKey);

void brinInclusionUnion(BrinDescriptor* brinDescriptor, BrinTupleValues* firstColumn, BrinTupleValues* secondColumn);
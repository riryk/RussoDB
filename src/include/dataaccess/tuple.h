#include "common.h"

#ifndef Tuple_h
#define Tuple_h

typedef struct TupleDescriptor
{
	uint attributesNumber;	
	Bool hasIdAttribute;
	uint referenceCount;
} *TupleDescriptor;

#endif






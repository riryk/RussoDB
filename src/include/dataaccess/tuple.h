#include "common.h"

#ifndef Tuple_h
#define Tuple_h

typedef struct TupleDescriptor
{
	uint attributesNumber;	
	Bool hasIdAttribute;
	uint referenceCount;
} *TupleDescriptor;

#define attributeIsNull(attribute, bits) (!((bits)[(attribute) >> 3] & (1 << ((attribute) & 0x07))))

#endif






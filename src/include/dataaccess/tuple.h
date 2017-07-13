#include "common.h"
//#include "rel.h"

#ifndef Tuple_h
#define Tuple_h

typedef struct TupleDescriptor
{
	uint attributesNumber;	
	//RelAttribute* attributes;
	Bool hasIdAttribute;
	uint referenceCount;
} *TupleDescriptor;

#define AttributeIsNull(attribute, bits) (!((bits)[(attribute) >> 3] & (1 << ((attribute) & 0x07))))

#endif






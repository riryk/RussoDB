#ifndef Tuple_h
#define Tuple_h

#include "common.h"
#include "relattribute.h"

typedef struct TupleDescriptor
{
	uint attributesNumber;	
	RelAttribute* attributes;
	Bool hasIdAttribute;
	uint referenceCount;
} *TupleDescriptor;

#define AttributeIsNull(attribute, bits) (!((bits)[(attribute) >> 3] & (1 << ((attribute) & 0x07))))

#define AttributeAlignNominal(currentOffset) ALIGN_SHORT(currentOffset)

#define FetchAttribute(attribute, tuple) FetchAttributeBase(tuple, (attribute)->byVal, (attribute)->len)

#define FetchAttributeBase(tuple, attributeByVal, attributeLen) \
( \
	(attributeByVal) ? \
	( \
		(attributeLen) == (int)sizeof(Datum) ? \
			*((Datum*)(tuple)) \
		: \
	  ( \
		(attributeLen) == (int)sizeof(int) ? \
			Int32GetDatum(*((int*)(tuple))) \
		: \
		( \
			(attributeLen) == (int)sizeof(int16) ? \
				Int16GetDatum(*((int16*)(tuple))) \
			: \
			( \
				CharGetDatum(*((char*)(tuple))) \
			) \
		) \
	  ) \
	) \
	: \
	PointerGetDatum((char*)(tuple)) \
)

#define AttributeAddLengthPointer(currentOffset, attributeLen, attributePointer) \
( \
	((attributeLen) > 0) ? \
	( \
		(currentOffset) + (attributeLen) \
	) \
	: (((attributeLen) == -1) ? \
	( \
		(currentOffset) + VarSizeAny(attributePointer) \
	) \
	: \
	( \
		(currentOffset) + (strlen((char*)(attributePointer)) + 1) \
	)) \
)

#endif






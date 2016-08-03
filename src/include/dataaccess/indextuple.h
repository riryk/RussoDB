#include "pageitempointer.h"

#ifndef Index_tuple_h
#define Index_tuple_h

typedef struct IndexTupleData
{
	PageItemPointerData PointerToTuple;		

	/* t_info is laid out in the following fashion:
	 *
	 * 15th (high) bit:      has nulls
	 * 14th bit:             has var-width attributes
	 * 13th bit:             unused
	 * 12-0 bit:             size of tuple
	 */
	unsigned short AdditionalInformation;		

} IndexTupleData;

typedef IndexTupleData *IndexTuple;


#endif






#ifndef BRIN_TUPLE_H
#define BRIN_TUPLE_H

#include "block.h"

typedef struct BrinTuple
{
	BlockNumber block;

	/* ---------------
	 * bt_info is laid out in the following fashion:
	 *
	 * 7th (high) bit: has nulls
	 * 6th bit: is placeholder tuple
	 * 5th bit: unused
	 * 4-0 bit: offset of data
	 * ---------------
	 */
	uint8 data;
} BrinTuple;


#endif
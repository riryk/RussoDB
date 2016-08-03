#include "common.h"

#ifndef BLOCK_H
#define BLOCK_H


/*
 * each data file (heap or index) is divided into disk blocks
 * (which may be thought of as the unit of i/o 
 *  A buffer contains exactly one disk block).  
 */
typedef uint BlockNumber;

#define InvalidBlockNumber ((BlockNumber) 0xFFFFFFFF)

#define MaxBlockNumber ((BlockNumber) 0xFFFFFFFE)

/*
 * BlockId:
 *
 * This is a storage type for BlockNumber. 
 * This type is used for on-disk structures (e.g., in HeapTupleData) 
 * whereas BlockNumber is the type on which calculations are performed 
 *
 * BlockIds can be SHORTALIGN'd (and therefore any
 * structures that contains them, such as ItemPointerData, can also be
 * SHORTALIGN'd).  this is an important consideration for reducing the
 * space requirements of the line pointer (ItemIdData) array on each
 * page and the header of each heap or index tuple.
 */
typedef struct BlockIdData
{
	uint16	    BlockIdHighPart;
	uint16		BlockIdLowPart;
} BlockIdData;

typedef BlockIdData *BlockId;

#endif

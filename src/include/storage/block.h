#include "common.h"

#ifndef BLOCK_H
#define BLOCK_H

typedef uint BlockNumber;

#define InvalidBlockNumber ((BlockNumber) 0xFFFFFFFF)

#define MaxBlockNumber ((BlockNumber) 0xFFFFFFFE)

typedef struct BlockIdData
{
	uint16	    BlockIdHighPart;
	uint16		BlockIdLowPart;
} BlockIdData;

typedef BlockIdData *BlockId;

#define SetBlockIdTo(blockId, blockNumber) \
( \
	(blockId)->BlockIdHighPart = (blockNumber) >> 16, \
	(blockId)->BlockIdLowPart = (blockNumber) & 0xffff \
)

#endif

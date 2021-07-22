#include <stdlib.h>

#define allocMinBits   3	
#define freeListNum    11

typedef struct AllocBlockData *AllocBlock;
typedef struct AllocChunkData *AllocChunk;

typedef struct AllocSetContext
{
	AllocBlock	blocks;			
	AllocChunk	freelist[freeListNum];
	size_t		initBlockSize;	
	size_t		maxBlockSize;	
	size_t		nextBlockSize;	
	size_t		allocChunkLimit;
	AllocBlock	keeper;	
	int			freeListIndex;
} AllocSetContext;

typedef AllocSetContext *AllocSet;

typedef struct AllocBlockData
{
	AllocSet	aset;	
	AllocBlock	prev;	
	AllocBlock	next;	
	char	   *freeptr;
	char	   *endptr;	
} AllocBlockData;

typedef struct AllocChunkData
{
	size_t		size;
	size_t		requested_size;
	void	   *aset;
} AllocChunkData;

typedef struct AllocSetFreeList
{
	int			     num_free;		
	AllocSetContext *first_free;
} AllocSetFreeList;

static AllocSetFreeList context_freelists[2] = {{0, NULL}, {0, NULL}};

int AllocSetFreeIndex(size_t size)
{
	int idx;
	if (size > (1 << allocMinBits))
	{
        idx = 31 - __builtin_clz((uint32)size - 1) - allocMinBits + 1;
	}

    return 0;
}




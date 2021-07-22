#include <stdlib.h>
#include <stdio.h>
#include "memnodes.h"
#include "memutils.h"

#define allocMinBits   3	
#define freeListNum    11

typedef struct AllocBlockData *AllocBlock;
typedef struct AllocChunkData *AllocChunk;

typedef struct AllocSetContext
{
	MemoryContextData header;
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

static void *AllocSetAlloc(MemoryContext context, size_t size);
static void AllocSetFree(MemoryContext context, void *pointer);
static void *AllocSetRealloc(MemoryContext context, void *pointer, size_t size);
static void AllocSetReset(MemoryContext context);
static void AllocSetDelete(MemoryContext context);
static size_t AllocSetGetChunkSpace(MemoryContext context, void *pointer);
static int AllocSetIsEmpty(MemoryContext context);

static const MemoryContextMethods AllocSetMethods = {
	AllocSetAlloc,
	AllocSetFree,
	AllocSetRealloc,
	AllocSetReset,
	AllocSetDelete,
	AllocSetGetChunkSpace,
	AllocSetIsEmpty
};

int AllocSetFreeIndex(size_t size)
{
	int idx;
	if (size > (1 << allocMinBits))
	{
        idx = 1; // 31 - __builtin_clz((int)size - 1) - allocMinBits + 1;
	}

    return idx;
}

MemoryContext AllocSetContextCreateInternal(MemoryContext parent,
							  const char *name,
							  int minContextSize,
							  int initBlockSize,
							  int maxBlockSize)
{
	AllocSet set;

	AllocSetFreeList *freelist = &context_freelists[1];
    if (freelist->first_free != NULL)
	{
        set = freelist->first_free;
        freelist->first_free = (AllocSet)set->header.nextchild;
		freelist->num_free--;

        set->maxBlockSize = maxBlockSize;

		MemoryContextCreate((MemoryContext)set,
							&AllocSetMethods,
							parent,
							name);
	}

    return (MemoryContext) set;
}

void *AllocSetAlloc(MemoryContext context, size_t size)
{
    return malloc(10);
}

void AllocSetFree(MemoryContext context, void *pointer)
{
}

void *AllocSetRealloc(MemoryContext context, void *pointer, size_t size)
{
    return malloc(10);
}

void AllocSetReset(MemoryContext context)
{
}

void AllocSetDelete(MemoryContext context)
{
}

size_t AllocSetGetChunkSpace(MemoryContext context, void *pointer)
{
	return 1;
}

int AllocSetIsEmpty(MemoryContext context)
{
}

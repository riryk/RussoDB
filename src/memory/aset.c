#include <stdlib.h>
#include <stdio.h>
#include "memnodes.h"
#include "memutils.h"

#define allocMinBits   3	
#define freeListNum    11

#define AllocBlockHeaderSize sizeof(AllocBlockData)
#define AllocChunkHeaderSize sizeof(struct AllocChunkData)

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
	int firstBlockSize;
	AllocSet set;
	AllocBlock block;
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

	    ((MemoryContext)set)->mem_allocated = set->keeper->endptr - ((char *)set);
        return (MemoryContext)set;
	}

	firstBlockSize = sizeof(AllocSetContext) 
		           + sizeof(AllocBlockData)
				   + sizeof(struct AllocChunkData);

	set = (AllocSet)malloc(firstBlockSize);
	if (set == NULL)
	{
        // throw error
	}

    block = (AllocBlock)(((char *)set) + sizeof(AllocSetContext));
	block->aset = set;
	block->freeptr = ((char *) block) + sizeof(AllocBlockData);
	block->endptr = ((char *) set) + firstBlockSize;
    block->prev = NULL;
	block->next = NULL;

    set->blocks = block;
	set->keeper = block;
    set->initBlockSize = initBlockSize;
	set->maxBlockSize = maxBlockSize;
	set->nextBlockSize = initBlockSize;

	MemoryContextCreate((MemoryContext)set,
							&AllocSetMethods,
							parent,
							name);

    return (MemoryContext) set;
}

void AllocSetReset(MemoryContext context)
{
	AllocSet set = (AllocSet)context;
    AllocBlock	block;
    block = set->blocks;
    
    while (block != NULL)
	{
        AllocBlock	next = block->next;

		if (block == set->keeper)
		{
			char* datastart = ((char *) block) + sizeof(AllocBlockData);

			memset(datastart, 0x7F, block->freeptr - datastart);

			block->freeptr = datastart;
			block->prev = NULL;
			block->next = NULL;
		}
		else
		{
			context->mem_allocated -= block->endptr - ((char *) block);
			memset(block, block->freeptr - ((char *) block));
			free(block);
		}

        block = next;
	}

	set->nextBlockSize = set->initBlockSize;
}

void AllocSetDelete(MemoryContext context)
{
    AllocSet set = (AllocSet)context;
	AllocBlock block = set->blocks;
	int keepersize = set->keeper->endptr - ((char *) set);

    if (set->freeListIndex >= 0)
	{
		AllocSetFreeList *freelist = &context_freelists[set->freeListIndex];
		if (freelist->num_free > 100)
		{
			while (freelist->first_free != NULL)
			{
			    AllocSetContext *oldset = freelist->first_free;
				freelist->first_free = freelist->first_free->header.nextchild;
				freelist->num_free--;
				free(oldset);
			}
		}

		set->header.nextchild = freelist->first_free;
		freelist->first_free = set;
		freelist->num_free++;
	}

	while (block != NULL)
	{
		AllocBlock next = block->next;

		if (block != set->keeper)
		{
			context->mem_allocated -= block->endptr - ((char*)block);
			free(block);
		}

		block = next;
	}

	free(set);
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

size_t AllocSetGetChunkSpace(MemoryContext context, void *pointer)
{
	return 1;
}

int AllocSetIsEmpty(MemoryContext context)
{
}

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "memnodes.h"
#include "memutils.h"

#define allocMinBits   3	
#define freeListNum    11

#define AllocBlockHeaderSize sizeof(AllocBlockData)
#define AllocChunkHeaderSize sizeof(struct AllocChunkData)

#define AllocPointerGetChunk(ptr)	\
   ((AllocChunk)(((char *)(ptr)) - AllocChunkHeaderSize))

#define AllocChunkGetPointer(chk)	\
   ((void*)(((char *)(chk)) + AllocChunkHeaderSize))


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
	int ind;
	int minChunkSize = (1 << allocMinBits);
    int chunksCount;
    int large;

	if (size <= minChunkSize)
		return 0;

	/* How many minChunks does the size contain? */
	chunksCount = (size - 1) >> allocMinBits;

	/* At this point we should calculate log2(chunksCount)
	 * We should check if chunksCount is more than 256 or not.
     * We divide it on 256 and see if the result is more than 0.
	 */
	large = chunksCount >> 8;
    
	/* chunksCount = large * (2^8) 
	 * And when we take log2(chunksCount) = 
	 * log2(large * (2^8)) = log2(large) + log2(2^8) = 
     * log2(large) + 8;
	 */
	if (large > 0)
        return log(large) / log(2) + 8;

    return log(chunksCount) / log(2);
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
			memset(block, 0x7F, block->freeptr - ((char *) block));
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
	AllocSet set = (AllocSet) context;
	AllocBlock block;
	AllocChunk chunk;

	int fidx;
	int chunk_size;
    int block_size;

	if (size > set->allocChunkLimit)  // size is bigger than maximum for chunks and allocate an entire block
	{
		chunk_size = size;  // round up to power of 2
        block_size = chunk_size + AllocBlockHeaderSize + AllocChunkHeaderSize;
		block = (AllocBlock)malloc(block_size);
		if (block == NULL)
		{
			return NULL;
		}

		context->mem_allocated += block_size;
        
		block->aset = set;
		block->freeptr = block->endptr = ((char*)block) + AllocBlockHeaderSize;

		chunk = (AllocChunk)(((char*)block) + AllocBlockHeaderSize);
		chunk->aset = set;
		chunk->size = chunk_size;
		chunk->requested_size = size;

		if (set->blocks != NULL)
		{
			block->prev = set->blocks;
			block->next = set->blocks->next;
            if (block->next != NULL)
			    block->next->prev = block;
			set->blocks->next = block;
		}
		else
		{
            block->prev = NULL;
			block->next = NULL;
            set->blocks = NULL;
		}

		return chunk;
	}
    
	fidx = AllocSetFreeIndex(size);
    chunk = set->freelist[fidx];
    if (chunk != NULL)
	{
		set->freelist[fidx] = (AllocChunk)chunk->aset;
		chunk->aset = (void*)set;
		chunk->requested_size = size;
		return chunk;
	}

	chunk_size = (1 << allocMinBits) << fidx;

	if ((block = set->blocks) != NULL)
	{
		size_t availspace = block->endptr - block->freeptr;
        if (availspace < (chunk_size + AllocChunkHeaderSize))
		{
            while (availspace >= ((1 << allocMinBits) + AllocChunkHeaderSize))
			{
                size_t availchunk = availspace - AllocChunkHeaderSize;
                fidx = AllocSetFreeIndex(availchunk);

				if (availchunk != ((size_t)1 << (fidx + allocMinBits)))
				{
                    fidx--;
					availchunk = ((size_t)1 << (fidx + allocMinBits));
				}

                chunk = (AllocChunk) (block->freeptr);
				block->freeptr += (availchunk + AllocChunkHeaderSize);
				availspace -= (availchunk + AllocChunkHeaderSize);

                chunk->size = availchunk;
                chunk->aset = (void*)set->freelist[fidx];
				set->freelist[fidx] = chunk;
			}

            /* Mark that we need to create a new block */
			block = NULL; 
		}
	}

	if (block == NULL)
	{
        int required_size;

		block_size = set->nextBlockSize;
		set->nextBlockSize <<= 1;
		if (set->nextBlockSize > set->maxBlockSize)
			set->nextBlockSize = set->maxBlockSize;

        required_size = chunk_size + AllocBlockHeaderSize + AllocChunkHeaderSize;
		while (block_size < required_size)
            block_size <<= 1;

        block = (AllocBlock)malloc(block_size);
        while (block == NULL && block_size > 1024 * 1024)
		{
            block_size >>= 1;
            if (block_size < required_size)
			   break;
            block = (AllocBlock)malloc(block_size);
		}

		if (block == NULL)
			return NULL;

		context->mem_allocated += block_size;
        
		block->aset = set;
		block->freeptr = ((char*)block) + AllocBlockHeaderSize;
		block->endptr = ((char*)block) + block_size;

        block->prev = NULL;
		block->next = set->blocks;
		if (block->next)
			block->next->prev = block;
		set->blocks = block;
	}
    
	chunk = (AllocChunk)(block->freeptr);
	block->freeptr += (chunk_size + AllocChunkHeaderSize);

	chunk->aset = (void*)set;
	chunk->size = chunk_size;
	chunk->requested_size = size;

    return chunk;
}

void AllocSetFree(MemoryContext context, void *pointer)
{
    AllocSet set = (AllocSet)context;
	AllocChunk chunk = AllocPointerGetChunk(pointer);

	if (chunk->size > set->allocChunkLimit)
	{
        AllocBlock block = (AllocBlock)(((char*)chunk) - AllocBlockHeaderSize);
        if (block->aset != set ||
			block->freeptr != block->endptr ||
			block->freeptr != ((char*)block) + (chunk->size + AllocBlockHeaderSize + AllocChunkHeaderSize))
		{
			// report error
		}

        if (block->prev != NULL)
			block->prev->next = block->next;
		else
			set->blocks = block->next;

		if (block->next != NULL)
			block->next->prev = block->prev;

		context->mem_allocated -= block->endptr - ((char*)block);
	}
	else
	{
		int fidx = AllocSetFreeIndex(chunk->size);
		chunk->aset = (void*)set->freelist[fidx];
		chunk->requested_size = 0;
		set->freelist[fidx] = chunk;
	}
}

void *AllocSetRealloc(MemoryContext context, void *pointer, size_t size)
{
    AllocSet set = (AllocSet)context;
	AllocChunk chunk = AllocPointerGetChunk(pointer);

	int oldsize = chunk->size;
	if (oldsize > set->allocChunkLimit)
	{
        AllocBlock block = (AllocBlock)(((char*)chunk) - AllocBlockHeaderSize);

		int chunkSize = max(size, set->allocChunkLimit + 1);
        int blockSize = AllocBlockHeaderSize + AllocChunkHeaderSize + chunkSize;
		int oldblocksize = block->endptr - ((char*)block);

		block = (AllocBlock)realloc(block, blockSize);
		if (block == NULL)
		  return NULL;

		context->mem_allocated -= oldblocksize;
		context->mem_allocated += blockSize;

		block->freeptr = block->endptr = ((char*)block) + blockSize;
        chunk = (AllocChunk)(((char*)block) + AllocBlockHeaderSize);

		if (block->prev != NULL)
			block->prev->next = block;
		else
			set->blocks = block;

		if (block->next != NULL)
			block->next->prev = block;
        
		chunk->size = chunkSize;

		return AllocChunkGetPointer(chunk);
	}
	else if (oldsize >= size)
	{
        int oldrequest = chunk->requested_size;
		chunk->requested_size = size;
        return pointer;
	}
	else
	{
        void *newPointer;
		newPointer = (void*)AllocSetAlloc(context, size);
        if (newPointer == NULL)
			return NULL;

		oldsize = chunk->requested_size;
        memcpy(newPointer, pointer, oldsize);
		AllocSetFree((MemoryContext)set, pointer);
	    return newPointer;
	}

    return malloc(10);
}

size_t AllocSetGetChunkSpace(MemoryContext context, void *pointer)
{
    AllocChunk chunk = AllocPointerGetChunk(pointer);
	int result = chunk->size + AllocChunkHeaderSize;
	return 1;
}

int AllocSetIsEmpty(MemoryContext context)
{
    if (context->isReset)
		return 1;
	return 0; 
}

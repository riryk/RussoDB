#include "memcontainermanager.h"

/* Calculates the number of a free list
 * depending on size. The free list should 
 * contain a memory chunk of length of a power of two.
 */
int calculateFreeListIndex(size_t   size)
{
	int			 ind;
	int          minChunkSize = (1 << MIN_CHUNK_POWER_OF_2);
    int          chunksCount;
    int          large;

	if (size <= minChunkSize)
		return 0;

	/* How many minChunks does the size contain? */
	chunksCount = (size - 1) >> MIN_CHUNK_POWER_OF_2;

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
        return Log2Table[large] + 8;

    return Log2Table[large];
}

void* allocSetAlloc(
    MemoryContainer          container, 
	size_t                   size)
{   
    MemorySet	 set = (MemorySet)container;
	MemoryBlock  block;
	MemoryChunk	 chunk;

    void*        chunkPtr;
    int			 freeListInd;

	size_t		 chunkSize;
	size_t		 blockSize;

	/* In this case the requested memory size 
	 * is more than the max chunk size. So that
	 * we can't allocate a memory chunk.
	 * In this case we allocate a new whole block.
	 */
	if (size > memset->chunkMaxSize)
	{
        chunkSize = ALIGN_DEFAULT(size);
		blockSize = chunkSize + MEM_BLOCK_SIZE + MEM_CHUNK_SIZE;
		block     = (MemorySet)malloc(blockSize);

		if (block == NULL)
		   ; /* Should report the error. Now it is not implemented. */
        
		block->memset    = set;
		block->freeStart = (char*)block + blockSize;
		block->freeEnd   = block->freeStart;

		chunk         = (MemoryChunk)((char*)block + MEM_BLOCK_SIZE);
		chunk->memset = set;
		chunk->size   = chunkSize;
		chunk->sizeRequested = size;
        
		chunkPtr = MemoryChunkGetPointer(chunk);

		/* If actual allocated size of memory is larger than
		 * the requested size we mark the "unused space".
		 */
		if (size < chunkSize)
            ((char*)chunkPtr)[size] = UNUSED_SPACE_MARK;

		/* Insert the block into the head of 
		 * the set's block list.
		 */ 
		if (set->blockList != NULL)
		{
			block->next = set->blockList->next;
			set->blockList->next = block;
		}
		else
		{
			block->next = NULL;
			set->blockList = block;
		}

		return chunkPtr;
	}

	/* The size of the chunk is too small to be an entire block.
	 * In this case we should treat it as a chunk.
	 * First of all we should look at the free list if 
	 * there is a free chunk that can come up to our 
	 * memory request.
	 */
    freeListInd = calculateFreeListIndex(size);
	chunk       = set->freelist[freeListInd];

	if (chunk != NULL)
	{
		/* We remove the chunk from the head of the free list. */
		set->freelist[freeListInd] = (MemoryChunk)chunk->memsetorchunk;

		/* MemSetOrChunk now points to the parent memory set */
		chunk->memsetorchunk = (void*)set;
        chunk->sizeRequested = size;		

        chunkPtr = MemoryChunkGetPointer(chunk);

        if (size < chunkSize)
            ((char*)chunkPtr)[size] = UNUSED_SPACE_MARK;

		return chunkPtr;
	}

	/* Now we should calculate the chunk size.
	 * freeListInd = log2(chunksCount)
	 * 2^(freeListInd) = 2^(log2(chunksCount))
	 * chunksCount = 2 ^ freeListInd;
	 * 1 << 10 = 2 ^ 10. So we should shift left 
	 * on freeListInd positions
	 * chunksCount = 1 << freeListInd;
	 * And then we should multiple on 2 ^ MIN_CHUNK_POWER_OF_2
	 */
	chunkSize = (1 << MIN_CHUNK_POWER_OF_2) << freeListInd;

	/* Take the first block in the blocks list */
	block = set->blockList;

	/* If block is not null we try to find out 
	 * if there is a free space on it which
	 * has size chunkSize.
	 */
	if (block != NULL)
	{
		size_t  availSpace = block->freeEnd - block->freeStart;

		if (availSpace < (chunkSize + MEM_CHUNK_SIZE))
		{
			/* The availiable space does not come up 
			 * for this chunk. But we can use this space in the future
			 * Now we carve up this space into chunks and add it 
			 * to the free list.
			 */

		}
	}
}


void* allocateSpace(
	MemoryContainer          container, 
	size_t                   size)
{
	container->isReset = False;

	return (*context->methods->alloc) (context, size);
}

MemoryContainer allocateNewMemCont(
	    MemoryContainer     parent,
		char*               name,
		size_t              minContainerSize,
	    size_t              initialBlockSize,
		size_t              maxBlockSize)
{
    
}
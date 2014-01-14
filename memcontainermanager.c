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
	if (size > set->chunkMaxSize)
	{
        chunkSize = ALIGN_DEFAULT(size);
		blockSize = chunkSize + MEM_BLOCK_SIZE + MEM_CHUNK_SIZE;
		block     = (MemorySet)malloc(blockSize);

		if (block == NULL)
		   ; /* Should report the error. Now it is not implemented. */
        
		block->memset    = set;
		block->freeStart = (char*)block + blockSize;
		block->freeEnd   = block->freeStart;

		chunk                = (MemoryChunk)((char*)block + MEM_BLOCK_SIZE);
		chunk->memsetorchunk = set;
		chunk->size          = chunkSize;
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
			size_t  minPosChunkSize = (1 << MIN_CHUNK_POWER_OF_2) + MEM_CHUNK_SIZE;

			/* The availiable space does not come up 
			 * for this chunk. But we can use this space in the future
			 * Now we carve up this space into chunks and add it 
			 * to the free list.
			 * Here availiable space is less than chunkSize and so
			 * less than MEMORY_CHUNK_MAX_SIZE.
			 */
            while (availSpace >= minPosChunkSize)
			{
                size_t   availChunkSize = availSpace - MEM_CHUNK_SIZE;
				int		 availFreeInd   = calculateFreeListIndex(availChunkSize);
                size_t   actualSpace    = (size_t)(1 << (availFreeInd + MIN_CHUNK_POWER_OF_2));

				if (availChunkSize != actualSpace)
				{
					availFreeInd--;
					availChunkSize = actualSpace;
				}

				chunk = (MemoryChunk)(block->freeStart);
                
				block->freeStart += (availChunkSize + MEM_CHUNK_SIZE);
                availSpace       -= (availChunkSize + MEM_CHUNK_SIZE); 
 
				chunk->size          = availChunkSize;
				chunk->sizeRequested = 0;		

				chunk->memsetorchunk        = (void*)set->freelist[availFreeInd];
				set->freelist[availFreeInd] = chunk;
			}

			block == NULL;
		}
	}

	/* If the actual active block does not contain enough
	 * free space for the chunk we should create a new block.
	 */
	if (block == NULL)
	{
        size_t		requiredSize;    

		/* Set blockSize. We keep track of all block sizes
		 * And we allocate blocks in increase order of their size.
		 * We start from initBlockSize and always double the blockSize
		 * until we reach maxBlockSize.
		 */
		blockSize = set->nextBlockSize;
        set->nextBlockSize << 1;
		if (set->nextBlockSize > set->maxBlockSize)
            set->nextBlockSize = set->maxBlockSize;

        /* We have a restriction for chunkSize:
		 * chunkSize < maxChunkSize
		 * We start blockSize from initialBlockSize
		 * Sometimes initialBlockSize can be less than maxChunkSize
		 * and so initialBlockSize < maxChunkSize.
		 * We double bloclSize until we reach the requiredSize.
		 */
		requiredSize = chunkSize + MEM_BLOCK_SIZE + MEM_CHUNK_SIZE;
		while (blockSize < requiredSize)
			blockSize <<= 1;

		/* Allocate a new block */
		block = (MemoryBlock)malloc(blockSize);
        
		if (block == NULL)
		    ; /* Out of memory. We should report an error. */

		block->memset    = set;
		block->freeStart = ((char*)block) + MEM_BLOCK_SIZE;
		block->freeEnd   = ((char*)block) + blockSize; 

		if (set->keeperBlock == NULL && blockSize == set->initBlockSize)
			set->keeperBlock = block;

		/* Insert the new block into the head of the freelist */
		block->next    = set->blockList;
		set->blockList = block;
	}

    /* do the allocation */
	chunk = (MemoryChunk)block->freeStart;

	block->freeStart += (chunkSize + MEM_CHUNK_SIZE);

	chunk->memsetorchunk = (void*)set;
	chunk->size = chunkSize;
	chunk->sizeRequested = size;		

    chunkPtr = MemoryChunkGetPointer(chunk);

    if (size < chunkSize)
       ((char*)chunkPtr)[size] = UNUSED_SPACE_MARK;

	return chunkPtr;
}

MemoryContainer memContCreate(
    MemoryContainer      container,
    MemContType          type, 
	size_t               size,
	MemoryContainer      parent,
	char*                name)
{
    MemoryContainer  newCont;
	size_t           neededSize = size + strlen(name) + 1;

    newCont = (container != NULL) ?
		(MemoryContainer)allocSetAlloc(container, neededSize) :
	    (MemoryContainer)malloc(neededSize);

    memset(newCont, 0, size);
    
	newCont->type      = type;
	newCont->parent    = NULL;
	newCont->childHead = NULL;
	newCont->next      = NULL;
	newCont->isReset   = True;
	newCont->name      = ((char*)type) + size;

	strcpy(newCont->name, name);

	if (parent != NULL)
	{
		newCont->parent   = parent;
		newCont->next     = parent->childHead;
		parent->childHead = newCont;
	}

	return newCont;
}



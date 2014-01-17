#include "memcontainermanager.h"
#include <stdio.h>

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

MemoryContainer allocMemContCreate(
	MemoryContainer    container,
    MemoryContainer    parent,
    char*              name,
	size_t             minContainerSize,
	size_t             initBlockSize,
	size_t             maxBlockSize)
{
	size_t       maxChunkSizeToBlock;
    size_t       blockSize;
	MemoryBlock  block;

	MemorySet    set = (MemorySet)memContCreate(
                                  container,
                                  MCT_MemorySet, 
	                              sizeof(SMemorySet),
	                              parent,
	                              name);

	initBlockSize = ALIGN_DEFAULT(initBlockSize);
	if (initBlockSize < 1024)
		initBlockSize = 1024;

	maxBlockSize = ALIGN_DEFAULT(maxBlockSize);
	if (maxBlockSize < initBlockSize)
		maxBlockSize = initBlockSize;

	set->initBlockSize = initBlockSize;
	set->maxBlockSize  = maxBlockSize;
	set->nextBlockSize = initBlockSize;

	set->chunkMaxSize  = MEMORY_CHUNK_MAX_SIZE;

	/* Calculate the max chunk size in comparison 
	 * to the max block size. 
	 * The chunk size limit is at most 1/8 of the max block size
	 * In the case when all chunks have the maximum size 
	 * only 1/8 of the block space will be wasted.
	 */
	maxChunkSizeToBlock = (maxBlockSize - MEM_BLOCK_SIZE) / MEMORY_CHUNK_MAX_SIZE_TO_BLOCK;
    
	/* There can be a situation when memory chunk max size is more than 
	 * the max chunk size to block. So we should syncronize this and
	 * we divide it on 2 until chunk max size becomes less than maxChunkSizeToBlock.
	 */
	while (set->chunkMaxSize + MEM_CHUNK_SIZE > maxChunkSizeToBlock)
		set->chunkMaxSize >>= 1;

    if (minContextSize <= MEM_BLOCK_SIZE + MEM_CHUNK_SIZE)    
		return (MemoryContainer)set;

	/* Here minContextSize is more than the block size.
	 * In this case we allocate the first block.
	 */
	blockSize = ALIGN_DEFAULT(minContextSize);
    block     = (MemoryBlock)malloc(blockSize);

	if (block == NULL)
		; /* An error has happened. We should report it. */

	block->memset    = set;
	block->freeStart = ((char*)block) + MEM_BLOCK_SIZE;
	block->freeEnd   = ((char*)block) + blockSize;

	/* Insert this block into the head of the blocks list. */
	block->next      = set->blockList;
	set->blockList   = block;
	set->keeperBlock = block;

	return (MemoryContainer)set;
}

/* Free the whole memory which has been allocated inside
 * a context and also inside its children.
 */
void resetMemContainer(MemoryContainer cont)
{
	MemoryContainer  child;

	/* If the context has children we first of all
	 * try to free memory from all children first.
	 */
	if (cont->childHead != NULL)   
	{
		/* Loop through the whole list of children and 
		 * call resetMemContainer recursively.
		 */
		for (child = cont->childHead; child != NULL; child = child->next)
		{
			resetMemContainer(child);
		}
    }

	if (!cont->isReset)
	{
        resetMemoryFromSet(cont);
        cont->isReset = True;
	}
}

void resetMemoryFromSet(MemorySet set)
{
	MemoryBlock  block;

    /* clear the free list first. */
	memset(set->freelist, 0, sizeof(set->freelist));
	block = set->blockList;
    
	/* Set new block list to NULL or to the keeper one. */
	set->blockList = set->keeperBlock;

    while (block != NULL)
	{
		MemoryBlock  next = block->next;

		if (block == set->keeperBlock)
		{
            char*  dataStart = ((char*)block) + MEM_BLOCK_SIZE;

			block->freeStart = dataStart;
			block->next      = NULL;

			break;
		}

		free(block);

		block = next;
	}

	/* Reset block size allocation sequence, too */
	set->nextBlockSize = set->initBlockSize;
}

void printSetStatistic(
    MemorySet       set, 
	int             level)
{
    MemoryBlock   block;
	MemoryChunk   chunk;
	long          nblocks;
	long          nchunks;
	long          totalSpace;
    long          freeSpace;
    int           ind;
	int           i;

	for (block = set->blockList; block != NULL; block = block->next)
	{
        nblocks++;
		totalSpace += block->freeEnd - ((char*)block);
		freeSpace  += block->freeEnd - block->freeStart;
	}

	/* Loop through all memory free lists */
    for (ind = 0; ind < MEMORY_SET_FREELISTS_NUM; ind++)
	{
		for (chunk = set->freelist[ind]; 
			 chunk != NULL; 
			 chunk = (MemoryChunk)chunk->memsetorchunk)       
		{
            nchunks++;
			freeSpace += chunk->size + MEM_CHUNK_SIZE;
		}
	}

    for (i = 0; i < level; i++)
		fprintf(stderr, "  ");

	fprintf(stderr, 
        "%s: %lu total in %ld blocks; %lu free (%ld chunks); %lu used\n",
		set->baseMemCont.name,
        totalSpace,
		nblocks,
		freespace,
		nchunks,
		totalspace - freespace);
}

void freeChunk(void* mem)
{
    MemoryChunk  chunk;
	MemorySet    set;
	int          ind;

	chunk  = (MemoryChunk)((char*)mem - MEM_CHUNK_SIZE);     
	set    = chunk->memsetorchunk;

	/* If the chunk's size is larger than chunk max size 
	 * we have allocated an entire block. So that we have 
	 * to find and free this block.
	 */
	if (chunk->size > set->chunkMaxSize)
	{
        /* Try to find the corresponding block first 
		 */
		MemoryBlock   block     = set->blockList;
        MemoryBlock   prevblock = set->blockList;

		while (block != null)
		{
            if (chunk == (MemoryChunk)((char*)block + MEM_BLOCK_SIZE))
				break;
			prevblock = block;
			block     = block->next;
		}

		if (block == NULL)
			; /* Could not find the block. We should report an error. */
        
		/* Remove the block from the block list */
		if (prevblock == NULL)
			set->blockList  = block->next;
		else
			prevblock->next = block->next;

		free(block);
		return;
	}

	/* Now we have a normal case and the chunk is small
	 * So we should return it to the free list.
	 */
	ind = calculateFreeListIndex(chunk->size);

	/* Insert into the head of the free list. */
	chunk->memsetorchunk = (void*)set->freelist[ind];
    set->freelist[ind]   = chunk;
}
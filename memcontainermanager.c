#include "memcontainermanager.h"
#include "ierrorlogger.h"
#include <stdio.h>

/* This table matches n to log2(n) + 1 
 * It has been created to avoid calculating it
 * all time.
 */
unsigned char Log2Table[256] =
{
	0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
	SeqOf16CopiesOf(5), SeqOf16CopiesOf(6), SeqOf16CopiesOf(6), SeqOf16CopiesOf(7), 
	SeqOf16CopiesOf(7), SeqOf16CopiesOf(7), SeqOf16CopiesOf(7),
	SeqOf16CopiesOf(8), SeqOf16CopiesOf(8), SeqOf16CopiesOf(8), SeqOf16CopiesOf(8), 
	SeqOf16CopiesOf(8), SeqOf16CopiesOf(8), SeqOf16CopiesOf(8), SeqOf16CopiesOf(8)
};

MemoryContainer  topMemCont     = NULL;
MemoryContainer  currentMemCont = NULL;
MemoryContainer  errorMemCont   = NULL;

FMalloc  funcMalloc  = NULL;
FFree    funcFree    = NULL;  
FRealloc funcRealloc = NULL;

typedef void* (*FRealloc)(void* mem, size_t size);

void ctorMemContMan(
    void*            self, 
	FMalloc          funcMallocParam,
	FFree            funcFreeParam,
	FRealloc         funcReallocParam)
{
	IMemContainerManager _         = (IMemContainerManager)self;
	IErrorLogger         elog      = _->errorLogger;

	ASSERT_ARG_VOID(elog, funcMalloc == NULL);
	ASSERT_ARG_VOID(elog, funcFree == NULL);
    ASSERT_ARG_VOID(elog, funcRealloc == NULL);

    funcMalloc  = funcMallocParam;
    funcFree    = funcFreeParam;  
	funcRealloc = funcReallocParam;

	ASSERT_ARG_VOID(elog, topMemCont == NULL);

    topMemCont = memSetCreate(_,
	                          NULL,
                              NULL,
                              "TopMemoryContainer",
	                          0,
	                          8 * 1024,
	                          8 * 1024);

    currentMemCont = topMemCont;
    
	/* Create error memory container.    
	 * We do not expect much memory to be allocated here.
	 * We have at least 8K memory to be preserved here
	 * to treat properly out of memory error.
	 */
	errorMemCont = memSetCreate(_,
		                        topMemCont,
								topMemCont,
								"ErrorContainer",
								8 * 1024,
                                8 * 1024,
	                            8 * 1024);
}

void resetErrCont(void* self)
{
    resetMemContainer(self, errorMemCont);
}

MemoryContainer changeCurrContainer(
	MemoryContainer    container)
{
	MemoryContainer oldCont = currentMemCont;

	currentMemCont = container;
	return oldCont;
}

MemoryContainer changeToErrorContainer()
{
    return changeCurrContainer(errorMemCont);
}

void dtorMemContMan(void*  self)
{
    IMemContainerManager  _    = (IMemContainerManager)self;
	IErrorLogger          elog = _->errorLogger;
    
	_->resetMemoryFromSet(_, topMemCont);

    ASSERT(elog, funcMalloc != NULL, NULL);
    ASSERT(elog, funcFree != NULL, NULL);
    ASSERT(elog, funcRealloc != NULL, NULL);

	funcFree(topMemCont);

    ASSERT(elog, topMemCont != NULL, NULL);
    ASSERT(elog, currentMemCont != NULL, NULL);
    ASSERT(elog, errorMemCont != NULL, NULL);

    funcMalloc    = NULL;
    funcFree      = NULL; 
    funcRealloc   = NULL;
	topMemCont    = NULL;
}

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

    return Log2Table[chunksCount];
}

/* This is an auxiliary function for allocateMemory method.
 * It allocates a whole new block, inserts it into 
 * the head of the set's block list 
 * and takes the whole memory from the entire block 
 * for a chunk.
 */
void* allocateChunkBlock(
    void*             self,
	size_t            chunkSize,
    MemoryContainer   container)
{
	IMemContainerManager _         = (IMemContainerManager)self;
	IErrorLogger         elog      = (IErrorLogger)_->errorLogger;
	size_t               blockSize = chunkSize + MEM_BLOCK_SIZE + MEM_CHUNK_SIZE;

	MemorySet	  set       = (MemorySet)container;
    MemoryBlock   block;
	MemoryChunk	  chunk;

    void*         chunkPtr;
    size_t        size      = ALIGN_DEFAULT(chunkSize);
    
    ASSERT(elog, funcMalloc != NULL, NULL);
    block = (MemoryBlock)funcMalloc(blockSize);

	/* Report malloc error */
	if (block == NULL)
	{
		showMemStat(_, topMemCont, 0);

		elog->log(LOG_ERROR, 
		          ERROR_CODE_OUT_OF_MEMORY, 
				  "Out of memory. Failed request size: %lu", 
				  chunkSize);
	}

	block->memset        = set;
	block->freeStart     = (char*)block + blockSize;
	block->freeEnd       = block->freeStart;

	chunk                = (MemoryChunk)((char*)block + MEM_BLOCK_SIZE);
	chunk->memsetorchunk = set;
	chunk->size          = size;
	chunk->sizeRequested = chunkSize;
    
	chunkPtr = MemoryChunkGetPointer(chunk);

	/* Insert the block into the head of 
	 * the set's block list.
	 */ 
	if (set->blockList != NULL)
	{
		block->next          = set->blockList->next;
		set->blockList->next = block;

		return chunkPtr;
	}
	
    block->next    = NULL;
	set->blockList = block;

	return chunkPtr;
}

/* This is also an auxiliary function for allocateMemory method.
 * It allocates a new block and the whole memory is free.
 * The difference between allocateBlock and allocateChunkBlock
 * is that allocateChunkBlock gives the whole memory to a chunk
 * and its free list is always empty. allocateBlock creates 
 * a new block and reclaims whole memory to its free list.
 */
void* allocateBlock(
    void*             self,
	size_t            chunkSize,
	MemoryContainer   container)
{
	IMemContainerManager  _     = (IMemContainerManager)self;
	IErrorLogger          elog  = _->errorLogger;

	MemorySet	          set   = (MemorySet)container;
	MemoryBlock           block;

    size_t		  requiredSize; 
	size_t        blockSize;
    
	ASSERT(elog, set->nextBlockSize > 0, NULL);
    ASSERT(elog, set->maxBlockSize > 0, NULL);

	/* Set blockSize. We keep track of all block sizes
	 * And we allocate blocks in increase order of their size.
	 * We start from initBlockSize and always double the blockSize
	 * until we reach maxBlockSize.
	 */
	blockSize = set->nextBlockSize;
    set->nextBlockSize <<= 1;
	if (set->nextBlockSize > set->maxBlockSize)
        set->nextBlockSize = set->maxBlockSize;

    /* Block size should at least cover the chunk */
	requiredSize = chunkSize + MEM_BLOCK_SIZE + MEM_CHUNK_SIZE;
	while (blockSize < requiredSize)
		blockSize <<= 1;

	/* Allocate a new block */
	ASSERT(elog, funcMalloc != NULL, NULL);
	block = (MemoryBlock)funcMalloc(blockSize);

	/* Report malloc error */
	if (block == NULL)
	{
		showMemStat(_, topMemCont, 0);

		elog->log(LOG_ERROR, 
		          ERROR_CODE_OUT_OF_MEMORY, 
				  "Out of memory. Failed request size: %lu", 
				  chunkSize);
	}

	block->memset    = set;
	block->freeStart = ((char*)block) + MEM_BLOCK_SIZE;
	block->freeEnd   = ((char*)block) + blockSize; 

	/* If the block size is an initial block size,
	 * we adjust a keeper block.
	 */
	// if (set->keeperBlock == NULL && blockSize == set->initBlockSize)
	//	set->keeperBlock = block;

	/* Insert the new block into the head of the freelist */
	block->next    = set->blockList;
	set->blockList = block;

	return block;
}

/* This is an auxiliary function for allocateMemory method. 
 * It checks if a block has enough free space 
 * to fit in a chunk. Also if it does not the function
 * splits up the free space on power of 2 chunks ands
 * them into freelist chunks array.
 */
Bool checkFreeBlockSpace(
    void*                 self,
	MemoryContainer       container, 
	MemoryBlock           block,
	size_t		          chunkSize)
{ 
    IMemContainerManager  _    = (IMemContainerManager)self;
	IErrorLogger          elog = _->errorLogger;

	MemorySet set = (MemorySet)container;
	/* First of all calculate free space 
	 * which is available in the block.
	 * block:  .....|.............|..........
	 *              ^             ^
	 *             /             /
	 *       freeStart       freeEnd
	 */
    size_t    availSpace   = block->freeEnd - block->freeStart;
	size_t    minPosChunkSize; 

	/* If available space is more than chunk size
	 * the block is valid and we return true.
	 */
	if (availSpace >= chunkSize + MEM_CHUNK_SIZE)
		return True;

	/* Minimum chunk size including the chunk header. */
	minPosChunkSize = (1 << MIN_CHUNK_POWER_OF_2) + MEM_CHUNK_SIZE;

	/* The availiable space does not fit to this chunk. 
	 * But we can use this space in the future
	 * Now we carve up this space into chunks 
	 * and add it to the free list.
	 * Here availiable space is less than chunkSize and so
	 * less than MEMORY_CHUNK_MAX_SIZE.
	 */
    while (availSpace >= minPosChunkSize)
	{
		MemoryChunk  chunk;
        size_t       availChunkSize = availSpace - MEM_CHUNK_SIZE;
		int		     availFreeInd   = calculateFreeListIndex(availChunkSize);
        size_t       actualSpace    = (size_t)(1 << (availFreeInd + MIN_CHUNK_POWER_OF_2));

		/* availChunkSize should always be a power of 2. */
		if (availChunkSize != actualSpace)
		{
			availFreeInd--;
            ASSERT(elog, availFreeInd > 0, NULL); 
			availChunkSize = (size_t)(1 << (availFreeInd + MIN_CHUNK_POWER_OF_2));
		}

		/* Take a chunk from the block's free space. */
		chunk = (MemoryChunk)(block->freeStart);
        
		/* Shorten the block's free space and move freeStart 
		 * pointer further to skip already allocated space 
		 */
		block->freeStart += (availChunkSize + MEM_CHUNK_SIZE);

		/* reduce available space for the chunk size. */
        availSpace       -= (availChunkSize + MEM_CHUNK_SIZE); 

		chunk->size          = availChunkSize;
		chunk->sizeRequested = 0;		

		/* Insert the chunk into free list. */
        chunk->memsetorchunk = (set->freelist[availFreeInd] != NULL) ?
			(void*)set->freelist[availFreeInd] :
		    chunk;

        set->freelist[availFreeInd] = chunk;
	}
    
	/* Here we have carved the free space up 
	 * into small power of 2 parts. But the block does not contain
	 * enough free space and we return false. 
	 */
    return False;
}

void* allocateMemory(
    void*                   self,
    MemoryContainer         container, 
	size_t                  size)
{   
	IMemContainerManager  _    = (IMemContainerManager)self;
	IErrorLogger          elog = _->errorLogger;
    
	Bool                  isContValid = MemoryContainerIsValid(container);
    Bool                  isSizeValid = MemAllocSizeIsValid(size);

	MemorySet	          set = (MemorySet)container;
	MemoryBlock           block;
	MemoryChunk	          chunk;

    void*                 chunkPtr;
    int			          freeListInd;

	size_t		          chunkSize;
	size_t		          blockSize;

	/* Assert if the container is valid. */
    ASSERT_ARG(elog, isContValid, NULL); 
	ASSERT(elog, set->chunkMaxSize > 0, NULL);

	if (!isSizeValid)
		elog->log(LOG_ERROR, -1, "Allocate memory request size: %lu is invalid", size);

	/* In this case the requested memory size 
	 * is more than the max chunk size. So that
	 * we can't allocate a memory chunk.
	 * In this case we allocate a new whole block.
	 */
	if (size > set->chunkMaxSize)
        return allocateChunkBlock(_, size, container);

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
		ASSERT(elog, chunk->size > size, NULL);

		/* We remove the chunk from the head of the free list. */
		set->freelist[freeListInd] = (MemoryChunk)chunk->memsetorchunk;

		/* MemSetOrChunk now points to the parent memory set */
		chunk->memsetorchunk = set;
        chunk->sizeRequested = size;		

        chunkPtr = MemoryChunkGetPointer(chunk);
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
    ASSERT(elog, chunkSize > size, NULL); 

	/* Take the first block in the blocks list */
	block = set->blockList;

	/* If the actual active block does not contain enough
	 * free space for the chunk we should create a new block.
	 */
	if (block == NULL || !checkFreeBlockSpace(_, set, block, chunkSize))
        block = allocateBlock(_, chunkSize, container);

    /* do the allocation */
	chunk = (MemoryChunk)block->freeStart;

	/* Move the block's free start pointer */
	block->freeStart += (chunkSize + MEM_CHUNK_SIZE);

	chunk->memsetorchunk = (void*)set;
	chunk->size          = chunkSize;
	chunk->sizeRequested = size;		

    chunkPtr = MemoryChunkGetPointer(chunk);
	return chunkPtr;
}

/* Creates new memory container object. 
 * container - a memory container where to allocate
 *             memory from.
 * parent    - If it is null the memory container 
 *             will be the root node.
 * type      - memory type
 * size      - the size of MemoryContainer. In our case it is equal to
 *             sizeof(MemokrySet). But if we in the future create a new 
 *             implementation class that derives from MemoryContainer, 
 *             we will have an opportunity to support it.
 * If container is null, we allocate memory from malloc
 * It can happen when we want to create the top memory context
 */
MemoryContainer memContCreate(
	void*                self,
    MemoryContainer      container,
	MemoryContainer      parent,
    MemContType          type, 
	size_t               size,
	char*                name)
{
	IMemContainerManager  _    = (IMemContainerManager)self;
	IErrorLogger          elog = _->errorLogger;

    MemoryContainer    newCont;
	size_t             neededSize = size + strlen(name) + 1;

	if (container != NULL)
	{
        newCont = (MemoryContainer)allocateMemory(self, container, neededSize);
	}
	else 
	{
        ASSERT(elog, funcMalloc != NULL, NULL); 
        newCont = (MemoryContainer)funcMalloc(neededSize); 
		ASSERT(elog, newCont != NULL, NULL); 
	}

    memset(newCont, 0, size);
    
	newCont->type      = type;
	newCont->parent    = NULL;
	newCont->childHead = NULL;
	newCont->next      = NULL;
	newCont->isReset   = True;
	newCont->name      = ((char*)newCont) + size;

	strcpy(newCont->name, name);

	if (parent != NULL)
	{
		newCont->parent   = parent;
		newCont->next     = parent->childHead;
		parent->childHead = newCont;
	}

	return newCont;
}

/* Creates new memory set object. */
MemorySet memSetCreate(
    void*              self,
	MemoryContainer    container,
    MemoryContainer    parent,
    char*              name,
	size_t             minContainerSize,
	size_t             initBlockSize,
	size_t             maxBlockSize)
{
	IMemContainerManager  _    = (IMemContainerManager)self;
	IErrorLogger          elog = _->errorLogger;

	size_t         blockMaxChunkSize;
    size_t         blockSize;
	MemoryBlock    block;

	/* MemorySet is derived from MemoryContainer.
	 * First we create a base object.
	 */
	MemorySet      set = (MemorySet)memContCreate(
		                          self,
                                  container,
								  parent,
                                  MCT_MemorySet, 
	                              sizeof(SMemorySet),
	                              name);

	ASSERT(elog, set != NULL, NULL);

	initBlockSize = ALIGN_DEFAULT(initBlockSize);

	/* Check if initBlockSize is less than 
	 * the minimum allowed value. 
	 */
	if (initBlockSize < MEM_BLOCK_INIT_MIN_SIZE)
		initBlockSize = MEM_BLOCK_INIT_MIN_SIZE;

	maxBlockSize = ALIGN_DEFAULT(maxBlockSize);

	/* maxBlock should be bigger than initBlockSize. */
	if (maxBlockSize < initBlockSize)
		maxBlockSize = initBlockSize;

	set->initBlockSize = initBlockSize;
	set->maxBlockSize  = maxBlockSize;
	set->nextBlockSize = initBlockSize;

	/* chunkMaxSize can't be more than chunk_max_size 
	 * because the number of free lists is restricted.
	 */
	set->chunkMaxSize  = MEMORY_CHUNK_MAX_SIZE;

	/* Calculate the max chunk size in comparison 
	 * to the max block size. 
	 * The chunk size limit is at most 1/8 of the max block size
	 * In the case when all chunks have the maximum size 
	 * only 1/8 of the block space will be wasted.
	 */
	blockMaxChunkSize = (maxBlockSize - MEM_BLOCK_SIZE) / MAX_BLOCK_CHUNKS_NUM;
    
	/* There can be a situation when memory chunk max size is more than 
	 * the max chunk size to block. So we should syncronize this and
	 * we divide it on 2 until chunk max size becomes less than maxChunkSizeToBlock.
	 */
	while (set->chunkMaxSize + MEM_CHUNK_SIZE > blockMaxChunkSize)
		set->chunkMaxSize >>= 1;

    if (minContainerSize <= MEM_BLOCK_SIZE + MEM_CHUNK_SIZE)    
		return (MemoryContainer)set;

	/* Here minContextSize is more than the block size.
	 * In this case we allocate the first block.
	 */
	blockSize = ALIGN_DEFAULT(minContainerSize);

	ASSERT(elog, funcMalloc != NULL, NULL); 
    block     = (MemoryBlock)funcMalloc(blockSize);

	/* An error has happened. We should report it. */
	if (block == NULL)
	{
        showMemStat(_, topMemCont, 0);

	    elog->log(LOG_ERROR, 
		          ERROR_CODE_OUT_OF_MEMORY, 
				  "Out of memory. Failed request size: %lu", 
				  blockSize);

		return NULL;
	}

	block->memset    = set;
	block->freeStart = ((char*)block) + MEM_BLOCK_SIZE;
	block->freeEnd   = ((char*)block) + blockSize;

	/* Insert this block into the head of the blocks list. */
	block->next      = set->blockList;
	set->blockList   = block;
	//set->keeperBlock = block;

	return (MemoryContainer)set;
}

void resetMemoryFromSet(
	void*              self,
	MemorySet          set)
{
    IMemContainerManager  _    = (IMemContainerManager)self;
	IErrorLogger          elog = _->errorLogger;

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

        ASSERT_VOID(elog, funcFree != NULL); 
		funcFree(block);

		block = next;
	}

	/* Reset block size allocation sequence, too */
	set->nextBlockSize = set->initBlockSize;
}

/* Free the whole memory which has been allocated inside
 * a context and also inside its children.
 */
void resetMemContainer(
    void*              self,
	MemoryContainer    cont)
{
	MemoryContainer    child;

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
			resetMemContainer(self, child);
		}
    }

	if (!cont->isReset)
	{
        resetMemoryFromSet(self, cont);
        cont->isReset = True;
	}
}

void showMemStat(
	void*             self,
    MemoryContainer   container,
	int               level)
{
    IMemContainerManager  _    = (IMemContainerManager)self;
	IErrorLogger          elog = _->errorLogger;
	MemoryContainer       child;

    Bool isContValid = MemoryContainerIsValid(container);

	ASSERT_ARG_VOID(elog, isContValid);

    printSetStatistic(container, level);
    
	for (child = container->childHead; child != NULL; child = child->next)
	{
		/* We should prevent an infinite loop. */
        ASSERT_VOID(elog, child != container);

        showMemStat(_, child, level + 1);    
	}
}

void printSetStatistic(
    MemorySet       set, 
	int             level)
{
    MemoryBlock   block;
	MemoryChunk   chunk;
	long          nblocks    = 0;
	long          nchunks    = 0;
	long          totalSpace = 0;
    long          freeSpace  = 0;
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
		freeSpace,
		nchunks,
		totalSpace - freeSpace);
}

/* Frees a piece of memory that have been allocated
 * as a chunk. The function converts this memory address
 * to a chunk and checks how this chunks has been allocated.
 * If it was like a full block we should free this block.
 * If is was an ordinary chunk we give it back to the freelist
 * for recycling.
 */
void freeChunk(
    void*          self,
	void*          mem)
{
    IMemContainerManager  _    = (IMemContainerManager)self;
	IErrorLogger          elog = _->errorLogger;

    MemoryChunk  chunk;
	MemorySet    set;
	int          ind;

	/* Get a pointer to MemoryChunk header. */
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
        MemoryBlock   prevblock = NULL;

		while (block != NULL)
		{
            if (chunk == (MemoryChunk)((char*)block + MEM_BLOCK_SIZE))
				break;

			prevblock = block;
			block     = block->next;
		}

		/* Could not find the block. We should report an error. */
		if (block == NULL)
		{
			 elog->log(LOG_ERROR, 
		          ERROR_CODE_BLOCK_NOT_FOUND, 
				  "Could not find block containing chunk %p", 
				  chunk);

			 return;
		}

		/* Remove the block from the block list */
		if (prevblock == NULL)
			set->blockList  = block->next;
		else
			prevblock->next = block->next;
        
		ASSERT_VOID(elog, funcFree != NULL); 
		funcFree(block);

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

/* Reallocate memory. When we need more memory
 * we allocate a piece of memory with an appropriate size,
 * copy an old memory and then free it.
 */
void* reallocateMemory(
    void*                  self,
	MemoryContainer        container, 
	void*                  old_mem, 
	size_t                 new_size)
{
	IMemContainerManager  _    = (IMemContainerManager)self;
	IErrorLogger          elog = _->errorLogger;
    
	MemorySet    set   = (MemorySet)container;
    MemoryChunk	 chunk = (MemoryChunk)((char*)old_mem - MEM_CHUNK_SIZE);  

	size_t		 oldsize = chunk->size;   
	void*        chunkPtr;
	void*        new_mem;

	/* Always return when a requested size is a decrease */
	if (oldsize >= new_size)
	    return old_mem;

	/* Check if for the chunk there was allocate a block */
	if (oldsize > set->chunkMaxSize)
	{
        /* Try to find the corresponding block first 
		 */
		MemoryBlock   block     = set->blockList;
        MemoryBlock   prevblock = NULL;

        size_t		  chunk_size;
		size_t		  block_size;
		char*         expected_block_end;

		while (block != NULL)
		{
            if (chunk == (MemoryChunk)((char*)block + MEM_BLOCK_SIZE))
				break;

			prevblock = block;
			block     = block->next;
		}

		/* Could not find the block. We should report an error. */
		if (block == NULL)
		{
			 elog->log(LOG_ERROR, 
		          ERROR_CODE_BLOCK_NOT_FOUND, 
				  "Could not find block containing chunk %p", 
				  chunk);

			 return NULL;
		}

        /* We should check that the chunk is only one 
		 * on the block.
		 */
		expected_block_end = (char*)block + 
			chunk->size +
            MEM_BLOCK_SIZE + 
			MEM_CHUNK_SIZE;

		ASSERT(elog, block->freeEnd == expected_block_end, NULL); 

		/* Do the realloc */
		chunk_size = ALIGN_DEFAULT(new_size);
		block_size = chunk_size + MEM_BLOCK_SIZE + MEM_CHUNK_SIZE;

		ASSERT(elog, funcRealloc != NULL, NULL); 
		block      = (MemoryBlock)funcRealloc(block, block_size);

		if (block == NULL)
		{
            showMemStat(_, topMemCont, 0);

	        elog->log(LOG_ERROR, 
		          ERROR_CODE_OUT_OF_MEMORY, 
				  "Out of memory. Failed request size: %lu", 
				  block_size);

		    return NULL;
		}

		block->freeStart = block->freeEnd = (char*)block + block_size;
        chunk = (MemoryChunk)((char*)block + MEM_BLOCK_SIZE);

		/* Change block pointer to newly allocated block. */
		if (prevblock == NULL)
			set->blockList = block;
		else
			prevblock->next = block;

		chunk->size = chunk_size;

		chunkPtr = MemoryChunkGetPointer(chunk);
	    return chunkPtr;
	}

	/* If we are here that this small block
	 * was taken from the free list. We allocate
	 * a new free chunk and the old chunk add to
	 * the free list. 
	 */
    new_mem = allocateMemory(_, container, new_size);

	/* copy existing memory to a new memory. */
	memcpy(new_mem, old_mem, oldsize);

	/* free old chunk */
	freeChunk(self, old_mem);
	return new_mem;
}


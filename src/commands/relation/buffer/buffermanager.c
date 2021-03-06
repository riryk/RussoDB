
#include "buffermanager.h"
#include "latch.h"
#include "latchmanager.h"
#include "imemorymanager.h"
#include "rel.h"

const SIBufferManager sBufferManager = 
{ 
	&sHashtableManager,
	&sRelRowManager,
    &sLatchManager,
    &sTrackMemManager
};

const IBufferManager bufferManager = &sBufferManager;


/* A hash table, which contains buffers loaded into memory. */
Hashtable    bufCache       = NULL;

/* An array with buffers additional information. 
 * To get a buffer from this array we need:
 *
 * 1. To find this buffer in bufCache
 * 2. Use the buffer's identifier id as an index in
 *    bufferInfos array.
 */
BufferInfo   bufInfos       = NULL;

/* PrivateRefUsageCounts is a number a buffer 
 * has been pinned inside the current process.
 * The benefits from this:
 *
 * 1. If we pin a buffer multiple times,
 *    we just increase the shared buffer's reference count
 *    and lock shared buffer state only once.
 * 2. When we abort a transaction, we just need 
 *    to unpin the buffer exactly the times we have pinned it.
 * 
 * If a buffer is being processed
 * the buffer's reference count is more than 0.
 * If a buffer is not being processed, 
 * the reference count is equal to 0.
 */
int*         bufPrivateRefUsageCounts = NULL;
char*        bufBlocks                = NULL;
int			 bufNum                   = 1000;

/* List of buffers which are available for reusing */
BufFreeListState     freeBufferState  = NULL;

/* A buffer which is in progress now.
 * A buffer is being flushed. */
BufferInfo           bufInProgress    = NULL;


void initFreeBufferState(void* self)
{
    IBufferManager      _    = (IBufferManager)self;
	IHashtableManager   hm   = _->hashtableManager;
	IMemoryManager      mm   = _->memoryManager;

	/* Calculate the buffer cache's size.
	 * bufNum is the maximum number of items which 
	 * can be inserted into the buffer cache.
	 * But bufferAlloc method tries to insert a new item
	 * and only after that it deletes the old item.
	 * Partitions number is the maximum number of concurrent 
	 * processes that work with the table.
	 */
    int bufCacheSize = bufNum + BUFFER_HASHTABLE_PARTITIONS;

	SHashtableSettings  set;
    memset(&set, 0, sizeof(SHashtableSettings)); 

	set.keyLen   = sizeof(SRelFileInfoBack);
	set.valLen   = sizeof(SRelData);
	set.hashFunc = hashFuncTag;
	set.partNum  = BUFFER_HASHTABLE_PARTITIONS;

    bufCache = hm->createHashtable(
			 hm,
             "shared buffer cache",
			 bufCacheSize,
			 &set,
			 HASH_FUNC | HASH_ITEM); 

	freeBufferState = (BufFreeListState)mm->alloc(sizeof(SBufFreeListState));
    
	/* We add the whole free list to the free buffer state. */
	freeBufferState->first = 0;
	freeBufferState->last  = bufNum - 1;

	freeBufferState->next            = 0;
	freeBufferState->completedCycles = 0;
	freeBufferState->bufferAllocated = 0;
	freeBufferState->latch           = NULL;
}

/* From the start all buffers are in free list.
 * The buffer ring looks like:
 * 
 * 0 -> 1 -> 2 -> .... -> n - 1 -> n -> (-1)
 */
void ctorBufMan(void* self)
{
	BufferInfo          buf;
	int			        i;

    IBufferManager      _    = (IBufferManager)self;

	IRelFileManager     rfm  = _->relFileManager;
	IHashtableManager   hm   = _->hashtableManager;
	IMemoryManager      mm   = _->memoryManager;

	rfm->ctorRelFileMan(rfm);
    
	bufInfos  = (BufferInfo)mm->alloc(bufNum * sizeof(SBufferInfo));
	bufBlocks = (char*)mm->alloc(bufNum * BlockSize);

	buf       = bufInfos;

	for (i = 0; i < bufNum; buf++, i++)
	{
		BUFFER_ID_DEFAULT(buf->id);

		buf->flags        = 0;
		buf->usageCount   = 0;
		buf->refCount     = 0;
		buf->backWaiterId = 0;
		buf->ind          = i;
		buf->freeNext     = i + 1; 
	}

	/* The last buffer in our linked free list points to nothing. */
	bufInfos[bufNum - 1].freeNext = FREENEXT_END_OF_LIST;

	initFreeBufferState(_);

	bufPrivateRefUsageCounts = (int*)mm->alloc(bufNum * sizeof(int));
	memset(bufPrivateRefUsageCounts, 0, bufNum * sizeof(int));
}

/*
 * StrategyRejectBuffer -- consider rejecting a dirty buffer
 *
 * When a nondefault strategy is used, the buffer manager calls this function
 * when it turns out that the buffer selected by StrategyGetBuffer needs to
 * be written out and doing so would require flushing WAL too.	This gives us
 * a chance to choose a different victim.
 *
 * Returns true if buffer manager should ask for a new victim, and false
 * if this buffer should be written and re-used.
 */
Bool strategyRejectBuffer(BufRing ring, BufferInfo buf)
{
	/* We only do this in bulkread mode */
	if (ring->type != BAS_BULKREAD)
		return False;

	/* Don't muck with behavior of normal buffer-replacement strategy */
	if (!ring->currentInRing ||
	     ring->buffers[ring->current] != buf->ind + 1)
		return False;

	/*
	 * Remove the dirty buffer from the ring; necessary to prevent infinite
	 * loop if all ring members are dirty.
	 */
	ring->buffers[ring->current] = -1;

	return True;
}

/* In the moment when we are processing a buffer we 
 * want to pin it so that no other processes will
 * touch it.
 */
Bool pinBuffer(
    void*                 self,
    BufferInfo            buf, 
	BufRing               ring)
{
	Bool res = True;
	int  i   = buf->ind;

    /* If ref usage count is more than 0, that means 
	 * this buffer is already pinned. In this case 
	 * we just increment the reference count.
	 */
	if (bufPrivateRefUsageCounts[i] > 0)
	{
        bufPrivateRefUsageCounts[i]++;
		return True;
	}

	/* In this case ref usage count is 0.
	 * And in this case the buffer is not pinned. 
	 * First of all we increment the buffer's refCount.
	 */
    buf->refCount++;

	 /* For a default strategy we increment usageCount. 
	  * For a different strategy we only keep
	  * usageCount more than 0.
	  */
    if (ring == NULL)
	{
        if (buf->usageCount < MAX_USAGE_COUNT)
            buf->usageCount++;
	}
	else
    {
		if (buf->usageCount == 0)
            buf->usageCount = 1;
	}

	bufPrivateRefUsageCounts[i]++;
	return (buf->flags & BUFFER_VALID) != 0;
}

/* The function marks buffer as available for using.
 * So that other processes can take it and 
 * start processing. */
void unpinBuffer(
	void*               self,
	BufferInfo          buf)
{
	int	  i = buf->ind;
    
	/* We decrement ref usage count. */
	bufPrivateRefUsageCounts[i]--;
    
	/* If after decrement of ref usage count
	 * the refcount is stille more than 0, 
	 * the buffer is still pinned.
	 */
	if (bufPrivateRefUsageCounts[i] == 0)
	{
		/* The buffer is unpinned. We decrement the buffer's ref count. */
		buf->refCount--;

		/* Flag BUFFER_PIN_WAITER means that there are 
		 * backends which are waiting for this buffer becomes 
		 * unpinned.
		 * If the buffer's reference count is 1 we clear
		 * BUFFER_PIN_WAITER from the buffer's flags.
		 */
		if ((buf->flags & BUFFER_PIN_WAITER) && buf->refCount == 1)
			buf->flags &= ~BUFFER_PIN_WAITER;	
	}
}

/* Finds a victim buffer in the buffer ring */
BufferInfo getBufferFromRingArray(BufRing ring)
{
	BufferInfo     buf;
	int		       num;

	/* Go to the next ring item.
	 * If we exceed the last item in the ring
	 * we wrap around and return to the start position.
	 */
	if (++ring->current >= ring->size)
		ring->current = 0;

	/* Return the next buffer to the caller. */
	num = ring->buffers[ring->current];

	/* num == -1 means that we have not selected this buffer. 
	 * So that this buffer is not filled and we return 0.
	 */
	if (num == -1)
	{
		ring->currentInRing = False;
		return NULL;
	}

	/* If the buffer is not pinned, we return it. */
	buf = &bufInfos[num - 1];
	if (buf->refCount == 0 && buf->usageCount <= 1)
	{
		ring->currentInRing = True;
		return buf;
	}

	/* If the buffer is pinned, we return null. */
	ring->currentInRing = False;
	return NULL;
}

/* This function adds the buf's index to 
 * the current buffer position in the ring
 */
void addBufferToRing(
	BufRing               ring,
	BufferInfo            buf)
{
	ring->buffers[ring->current] = buf->ind + 1;
}

BlockNumber RelationGetNumberOfBlocksInFork(Relation relation, FilePartNumber forkNumber)
{
	RelationOpenFile(relation);

	return getBlocksNumInRelPart(NULL, relation->data, forkNumber);
}

uint getBlocksNumInRelPart(
	void*                 self,
	RelData               rel, 
	FilePartNumber        pnum)
{
	IBufferManager   _        = (IBufferManager)self;
	IRelFileManager  rfm      = _->relFileManager;

	return rfm->getBlocksNum(rfm, NULL, rel, pnum, REL_SEGM_SIZE);
}

/* This function finds a victim buffer
 * taking into account the strategy.
 * ring is used when we do a sequential scan 
 * or vacuum operations. In this case we know 
 * exactly a buffer's usage count and we do not 
 * need an optimization by "buffer clockwise algorithm".
 */
BufferInfo getBufferFromRing(
	void*                 self,
	BufRing               ring)
{
	IBufferManager   _             = (IBufferManager)self;
	ILatchManager    latchMan      = _->latchManager;

	BufferInfo       buf;
	int			     counter;
	Latch            latch;

	/* First of all try to get buffer from the ring */
    if (ring != NULL)
	{
		buf = _->getBufferFromRingArray(ring);
		if (buf != NULL)
			return buf;
	}     

	/* If there is a latch in the free list,
	 * we should notify a vaiting process. 
	 */
	latch = freeBufferState->latch;
	if (latch != NULL)
	{
		freeBufferState->latch = NULL;
		latchMan->setLatch(latch);
	}

    /* Try to get a buffer from the free list 
	 * Here we should describe the algorithm:
	 * Frome start we have all buffers in the free list.
	 * In the future some of the buffers can be removed 
	 * and some added. We go through the whole free list
	 * and try to take out an unpinned buffer.
	 * It is possible that all the buffers in the free list
	 * are pinned and we can not find a buffer.
	 * In this case we move over to victim buffer selection algorithm.
	 */
	while (freeBufferState->first >= 0)
	{
		/* Get the head buffer from  */
		buf = &bufInfos[freeBufferState->first];

		/* Remove buffer from freelist 
		 * Free list is organised thus:
		 * freeBuffers->first->1->2->3->...->n-1->n
		 * freeBuffers->first is the head of the free list.
		 * So that when we delete the first item:
		 * freeBuffers->first is 1.
		 * 1. move next pointer to the next free item 2
		 *                   /--\
		 *                  /    V
		 * freeBuffers->first 1->2->3->...->n-1->n 
		 * 2. But after that buffer 1 still points to buffer 2
		 * We need to remove this link.
		 *                   /--\
		 *                  /    V
		 * freeBuffers->first 1  2->3->...->n-1->n 
		 */
		freeBufferState->first = buf->freeNext;
		buf->freeNext = FREENEXT_NOT_IN_LIST; 

		/* If the buffer is pinned we can't use it, we just skip it
		 * and run the code above again to find another buffer
		 */
		if (buf->refCount == 0 && buf->usageCount == 0)
		{
			/* Here the buffer is not pinned and we add it 
			 * to the buffer ring
			 */
			if (ring != NULL)
                addBufferToRing(ring, buf);

			/* Return this buffer to the caller  */
			return buf;
		}
	}

	/* We have not found any items in the free list.
	 * Now we try to find a free item using 
	 * "clockwise algorithm". 
	 * This algorithm is based on a buffer's usage count.
	 * When we pin a buffer we increment a buffer's usage count.
	 * Let's imagine that we have 5 sequential pins for this buffer
	 * After that the buffer's usage count is 5.
	 * And we have pinned some other buffer 3 times. So that the first
	 * buffer is more popular than the second. And during "clockwise algorithm"
	 * it should survive more passes.
	 * We read buffers from 0 to bufNum - 1 and we if we reach the last
	 * item we return to the first item.
	 * If we find an unpinned buffer with usage count equal to 0,
	 * we stop the algorithm and return the buffer.
	 * If we find an unpinned buffer with usage count more than 0
	 * we should do one full cycle more to be able to come back to this buffer. 
	 */
	counter = bufNum;

	CYCLE
	{
		buf = &bufInfos[freeBufferState->next];

		/* If we have reached the last item in the array, 
		 * we wrap around and return to the start of the array. 
		 */
		if (++freeBufferState->next >= bufNum)
		{
            freeBufferState->next = 0;
			freeBufferState->completedCycles++;
		}

		/* If the buffer is not pinned (buf->refCount == 0) 
		 * we can take this buffer. */
		if (buf->refCount == 0)
		{
			/* If usage count is more than 0, 
			 * we decrement usage count and start the alrorithm again. */
			if (buf->usageCount > 0)
			{
				buf->usageCount--;
				counter = bufNum;
				continue;
			}
			
		    /* If we have found a buffer, we just add it to the ring */
			if (ring != NULL)
				addBufferToRing(ring, buf);

			return buf;
		}

		/* If we have not found a buffer, 
		 * we decrement the counter. 
		 * We the counter becomes 0 that means we have scanned 
		 * all buffers and it all are pinned
		 * Here we throw an exception.
		 */
		if (--counter == 0)
			; /* We need to throw an exception. */
	}
}

/* This method physically writes a buffer out.
 * This method does not actually writes a buffer to disk
 * It only passes it to the operational system's kernel.
 * Actual write will be realised later when the system decides apon that.
 * It is acceptable because we have written logs before.
 * The buffer should be pinned because we should prohibit 
 * other backends from changing the buffer's inward structure.
 */
void flushBuffer(
		   void*          self,
		   BufferInfo     buf, 
		   RelData        rel)
{
	IBufferManager     _          = (IBufferManager)self;
	IRelFileManager    rfm        = _->relFileManager;
	//IRelationManager   rm         = _->relationManager;
    ICommon            com        = _->commonManager;

	/* We should keep a reference to a buffer 
	 * which is being flushed. */
	bufInProgress = buf;

    /* Open a relation which this buffer belongs to */
	//if (rel == NULL)
	//	rel = rm->openRelation(rm, &(buf->id.relId), -1);

    /* buffer just dirtied flag means that some moment ago
	 * this buffer has been clean and someone has just dirtied it.
	 * Let's illustrate what the following operation means:
	 *  buf->flags    = 0000 0000 0000 0000 
	 *  BUF_JUST_DIRT = 0000 0001 0000 0000
	 * ~BUF_JUST_DIRT = 1111 1110 1111 1111
	 * So that the following operation clears 9th bit from buf-flags
	 * bit array.
	 * We clear JUST_DIRTIED flag which means that the buffer is clean.
	 */
    buf->flags &= ~BUFFER_JUST_DIRTIED;

	/* bufBlocks is a memory which includes all blocks.
	 * If we have a block's index we can calculate the block's position
	 * inside this memory: (index * block_size, (index + 1) * block_size)
	 * call os's write function.
	 */
	rfm->writeBlock(
              rfm,
			  NULL,
			  rel,
              buf->id.relPart,
			  buf->id.blockNum,
			  (void*)(bufBlocks + ((size_t)buf->ind) * BlockSize));

	/* Clear flags buffer io is progress and buffer error */
	buf->flags &= ~(BUFFER_IO_IN_PROGRESS | BUFFER_IO_ERROR);

    /* Someone can dirty the buffer and set buffer just dirtied flag again.
	 * In this case we do not clear buffer_dirty flag and it will
	 * be flushed later. Otherwise the buffer is clear and we delete
	 * buffer_dirty and buffer_checkpoint flags
	 */
	if (!(buf->flags & BUFFER_JUST_DIRTIED))
		buf->flags &= ~(BUFFER_DIRTY | BUFFER_CHECKPOINT_NEEDED);
}

Bool startBufferWriting(
    void*                 self,
	BufferInfo            buf)
{
    if (buf->flags & BUFFER_IO_IN_PROGRESS)
	   ; /* Wait for other backend to complete IO */

	/* We are not able to start buffer IO, because some
	 * backend has already started it. 
	 */
	if (buf->flags & BUFFER_VALID)
	   return False;

	buf->flags |= BUFFER_IO_IN_PROGRESS;
	bufInProgress = buf;

	return True;
}

/* Get a buffer from the buffer cache. 
 * If the buffer has not been found in the cache
 * we create a new buffer and add it to the cache
 */
BufferInfo allocateBuffer(
		   void*                 self,
		   RelData               rel,
		   FilePartNumber        partNum,
		   uint                  blockNum,
		   BufRing               ring,
		   Bool*                 found)
{
	IBufferManager     _          = (IBufferManager)self;
    IHashtableManager  hashMan    = _->hashtableManager;

	SBufferId	       bufId;
	BufCacheItem       bufIt;
    BufferInfo         buf;
	Bool               valid;
	uint16             flags;

	/* Initialize the buffer's identifier.
	 * The identifier is 
	 * (relation id, relation part, block number)
	 */
	bufId.relId    = rel->relKey.node;
	bufId.relPart  = partNum;
    bufId.blockNum = blockNum;
    
    /* Try to find this buffer in the cache. */
	bufIt = (BufCacheItem)hashMan->hashFind(bufCache, (void*)&bufId);

	/* When bufIt != NULL, we have found the buffer. 
	 * bufIt->id is the buffer's identifier by which we can
	 * find it in local buffer array.
	 */
	if (bufIt != NULL)
	{
		/* Get the buffer's info from the buffer array. */
		buf = &bufInfos[bufIt->id];
		_->pinBuffer(_, buf, ring);
		return buf;
	}

	/* We have not found the buffer in the cache.
	 * We have to retrieve a victim buffer from the
	 * buffer pool. 
	 */
	CYCLE
	{
		int    bnum;

        /* Get a victim buffer to replace */
        buf   = getBufferFromRing(_, ring);
		bnum  = buf->id.blockNum;
		flags = buf->flags;

		/* Pin this buffer by incrementing the buffer's ref count. */
		if (bufPrivateRefUsageCounts[bnum] == 0)
		    buf->refCount++;

		/* Also we increment the private reference count. */
	    bufPrivateRefUsageCounts[bnum]++;

		/* If the buffer is dirty, we try to write it out. */
		if (flags & BUFFER_DIRTY)    
			_->flushBuffer(_, buf, NULL);
        
        bufIt = (BufCacheItem)hashMan->hashFind(bufCache, (void*)&bufId);
        
		/* If we have not found the buffer in the buffer cache we insert it
		 * We insert only the buffer's index. 
		 */
		if (bufIt == NULL)
		{
		    bufIt = (BufCacheItem)hashMan->hashInsert(hashMan, bufCache, (void*)&bufId);
			bufIt->id = buf->ind;

			/* While we were making the flush or inserting the buffer
			 * into the buffer cache someone else coule have pinned or
			 * redirtied it. In this case we can't use this buffer.
			 * If the buffer is clean and not pinned we break from 
			 * the cycle because we have found the buffer we needed.
			 */
			if (buf->refCount == 1 && !(buf->flags & BUFFER_DIRTY))
			    break;

			/* If we are here it means that the buffer is dirty or pinned.
			 * And thus we can not use it. In this case we remove it from the
			 * cache and start the cycle again.
			 */
			hashMan->hashRemove(hashMan, bufCache, (void*)&bufId);
			_->unpinBuffer(_, buf);
		}

		/* Here bufIt != NULL. So we have found the buffer in the buffer cache.
		 * Someone else has already inserted it. In this case we just 
		 * return the buffer.
		 */
        buf = &bufInfos[bufIt->id];
		return buf;
	}

	buf->usageCount = 1;

	/* We have found a victim buffer and if this buffer is 
	 * present in the buffer cache, we will remove it.
	 */
	if (buf->flags & BUFFER_VALID)
		hashMan->hashRemove(hashMan, bufCache, (void*)&(buf->id));

	/* When startBufferWriting returns true, the buffer already
	 * contains the page and we do not need to load it from the
	 * database. 
	 * found means that the page has already been loaded on the buffer.
	 */
    *found = !startBufferWriting(_, buf);

	return buf;
}

int BufferGetBlockNumber(int buffer)
{
	BufferInfo   bufInf = &(bufInfos[buffer - 1]);

  	return bufInf->id.blockNum;
}

void dirtyBuffer(int buffer)
{
    BufferInfo   bufInf = &(bufInfos[buffer - 1]);

	bufInf->flags |= (BUFFER_DIRTY | BUFFER_JUST_DIRTIED);
}

BufferId ReadBuffer(Relation reln, BlockNumber blockNum)
{
	return 0;
}

void ReleaseBuffer(int buffer)
{
	
}

int readBufferInternal(
	      void*                  self,
		  RelData                rel, 
		  FilePartNumber         partnum,
		  uint                   blocknum,
		  BufRing                ring)
{
	IBufferManager    _      = (IBufferManager)self;
	IRelFileManager   rfm    = _->relFileManager; 
    //IRelationManager  rm     = _->relationManager;

	void*           bufBlock;
	BufferInfo      bufHdr;
	Bool            found;
	Bool		    isExtend = blocknum == BUFFER_NEW;

	//if (rel == NULL)
	//	rel = rm->openRelation(rm, &(rel->relKey.node), -1);

    if (isExtend)
        blocknum = rfm->getBlocksNum(rfm, "", rel, partnum, REL_SEGM_SIZE);

	bufHdr = allocateBuffer(
		        self,
		        rel,
		        partnum,
		        blocknum,
		        NULL,
				&found);

    if (found && !isExtend)
		return bufHdr->ind + 1;
    
	/*
	 * if we have gotten to this point, we have allocated a buffer for the
	 * page but its contents are not yet valid. We need to load its 
	 * content from the database.
	 */
    if (isExtend)
	{
		memset((char*)bufBlocks, 0, BlockSize);

		rfm->extendRelation(rfm, "", rel, partnum, blocknum, (char*)bufBlock);
	}
	else
		rfm->readBlock(rfm, "", rel, partnum, blocknum, (char*)bufBlock);

	bufHdr->flags |= BUFFER_VALID;

	bufBlock = (void*)(bufBlocks + ((size_t)bufHdr->ind) * BlockSize);
    return bufHdr->ind + 1;
}


/* 
 * Information saved between calls so we can determine the strategy
 * point's advance rate and avoid scanning already-cleaned buffers.
 */
static    Bool    saved_info_valid = False;
static    int	  prev_strategy_buf_id;
static    uint    prev_strategy_passes;
static    int	  next_to_clean;
static    uint    next_passes;

/* Moving averages of allocation rate and clean-buffer density */
static    float   smoothed_alloc = 0;
static    float   smoothed_density = 10.0;

/* Write out some dirty buffers in the pool.
 * This is called periodically by the background writer process.
 *
 * Returns true if it's appropriate for the bgwriter process to go into
 * low-power hibernation mode.	(This happens if the strategy clock sweep
 * has been "lapped" and no buffer allocations have occurred recently)
 */
Bool backgroundSyncBuffers()
{
	int         strategy_buf_id;
	uint        strategy_passes;
	uint        recent_alloc;
	long        strategy_delta;
	int	        bufs_to_lap;
	int			bufs_ahead;
	float	    scans_per_alloc;
	int			min_scan_buffers;
	int			BgWriterDelay = 200;

	/* Potentially these could be tunables, but for now, not */
	float	    smoothing_samples = 16;
    float		scan_whole_pool_milliseconds = 120000.0;

	int		    reusable_buffers_est;
	int			upcoming_alloc_est;

	strategy_passes   = freeBufferState->completedCycles;
	recent_alloc      = freeBufferState->bufferAllocated;
	strategy_buf_id   = freeBufferState->next;

    /* Compute strategy_delta = how many buffers have been scanned by the
	 * clock sweep since last time. */
    if (saved_info_valid)
	{
	    int   passes_delta = strategy_passes - prev_strategy_passes;
        
		strategy_delta = strategy_buf_id - prev_strategy_buf_id;
		strategy_delta += (long) passes_delta * bufNum;

		if ((int)(next_passes - strategy_passes) > 0)
		{
			/* we're one pass ahead of the strategy point */
		    bufs_to_lap = strategy_buf_id - next_to_clean;
		}
		else if (next_passes == strategy_passes &&
				 next_to_clean >= strategy_buf_id)
		{
            /* on same pass, but ahead or at least not behind */
			bufs_to_lap = bufNum - (next_to_clean - strategy_buf_id);
		}
        else
		{
			/*
			 * We're behind, so skip forward to the strategy point and start
			 * cleaning from there.
			 */
            next_to_clean = strategy_buf_id;
			next_passes   = strategy_passes;
			bufs_to_lap   = bufNum;
		}
	}
	else
	{
        /*
		 * Initializing at startup or after LRU scanning had been off. Always
		 * start at the strategy point.
		 */
        strategy_delta = 0;
		next_to_clean  = strategy_buf_id;
		next_passes    = strategy_passes;
		bufs_to_lap    = bufNum;
	}

    /* Update saved info for next time */
	prev_strategy_buf_id = strategy_buf_id;
	prev_strategy_passes = strategy_passes;
	saved_info_valid = True;

    /*
	 * Compute how many buffers had to be scanned for each new allocation, ie,
	 * 1/density of reusable buffers, and track a moving average of that.
	 *
	 * If the strategy point didn't move, we don't update the density estimate
	 */
    if (strategy_delta > 0 && recent_alloc > 0)
    {
        scans_per_alloc = (float)strategy_delta / (float)recent_alloc;
		smoothed_density += (scans_per_alloc - smoothed_density) /
			smoothing_samples;       
	}

	/*
	 * Estimate how many reusable buffers there are between the current
	 * strategy point and where we've scanned ahead to, based on the smoothed
	 * density estimate.
	 */
	bufs_ahead = bufNum - bufs_to_lap;
	reusable_buffers_est = (float) bufs_ahead / smoothed_density;

	/*
	 * Track a moving average of recent buffer allocations.  Here, rather than
	 * a true average we want a fast-attack, slow-decline behavior: we
	 * immediately follow any increase.
	 */
	if (smoothed_alloc <= (float) recent_alloc)
		smoothed_alloc = recent_alloc;
	else
		smoothed_alloc += ((float) recent_alloc - smoothed_alloc) /
			smoothing_samples;

	/* Scale the estimate by a GUC to allow more aggressive tuning. */
	upcoming_alloc_est = (int) (smoothed_alloc * 2.0);

    if (upcoming_alloc_est == 0)
		smoothed_alloc = 0;

	/*
	 * Even in cases where there's been little or no buffer allocation
	 * activity, we want to make a small amount of progress through the buffer
	 * cache so that as many reusable buffers as possible are clean after an
	 * idle period.
	 *
	 * (scan_whole_pool_milliseconds / BgWriterDelay) computes how many times
	 * the BGW will be called during the scan_whole_pool time; slice the
	 * buffer pool into that many sections.
	 */
	min_scan_buffers = (int)(bufNum / (scan_whole_pool_milliseconds / BgWriterDelay));

	if (upcoming_alloc_est < (min_scan_buffers + reusable_buffers_est))
	{
		upcoming_alloc_est = min_scan_buffers + reusable_buffers_est;
	}

	/*
	 * Now write out dirty reusable buffers, working forward from the
	 * next_to_clean point, until we have lapped the strategy scan, or cleaned
	 * enough buffers to match our estimate of the next cycle's allocation
	 * requirements, or hit the bgwriter_lru_maxpages limit.
	 */
}

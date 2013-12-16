
#include "buffermanager.h"
#include "latch.h"
#include "latchmanager.h"

const SIBufferManager sBufferManager = 
{ 
	&sHashtableManager,
	&sRelRowManager,
    &sLatchManager
};

const IBufferManager bufferManager = &sBufferManager;

Hashtable    relStorageHash = NULL;

/* A hash table, which contains buffers loaded into memory. */
Hashtable    bufCache  = NULL;

/* An array with buffers additional information. 
 * To get a buffer from this array we need:
 * 1. To find this buffer in bufCache
 * 2. Use the buffer's identifier id as an index in
 *    bufferInfos array.
 */
BufferInfo   bufInfos          = NULL;

/* If a buffer is being processed
 * the buffer's reference count is more than 0.
 * If a buffer is not being processed, 
 * the reference count is equal to 0.
 */
int*         bufRefUsageCounts = NULL;

char*        bufferBlocks;

int			 NBuffers = 1000;

/* Pointers to shared state */
BufferStrategyControl StrategyControl = NULL;

/* local state for StartBufferIO and related functions */
BufferInfo  InProgressBuf = NULL;
Bool        IsForInput;

/*
 * ReadBuffer -- a shorthand for ReadBufferExtended, for reading from main
 *		fork with RBM_NORMAL mode and default strategy.
 */
/*int readBuffer(Relation reln, int blockNum)
{
	return ReadBufferExtended(reln, MAIN_FORKNUM, blockNum, RBM_NORMAL, NULL);
}*/

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
Bool strategyRejectBuffer(BufferAccessStrategy strategy, BufferInfo buf)
{
	/* We only do this in bulkread mode */
	if (strategy->btype != BAS_BULKREAD)
		return False;

	/* Don't muck with behavior of normal buffer-replacement strategy */
	if (!strategy->current_was_in_ring ||
	     strategy->buffers[strategy->current] != buf->bufInd + 1)
		return False;

	/*
	 * Remove the dirty buffer from the ring; necessary to prevent infinite
	 * loop if all ring members are dirty.
	 */
	strategy->buffers[strategy->current] = -1;

	return True;
}


RelData openRelation(
    void*          self,
	SRelFileInfo   fileInfo, 
	int            backend)
{
	IBufferManager     _             = (IBufferManager)self;
	IHashtableManager  hashMan       = _->hashtableManager;
    SRelFileInfoBack   fileInfoBack;
    RelData            rel;

    if (relStorageHash != NULL)
	{
		SHashtableSettings  set;
        memset(&set, 0, sizeof(SHashtableSettings));

		set.keyLen = sizeof(SRelFileInfoBack);
		set.valLen = sizeof(SRelData);
		set.hashFunc = hashFuncTag;

		relStorageHash = hashMan->createHashtable(
			 hashMan,
             "relation storage table",
			 400,
			 &set,
			 HASH_FUNC | HASH_ITEM);
	}

	fileInfoBack.node    = fileInfo; 
	fileInfoBack.backend = backend;

	rel = (RelData)hashMan->hashFind(
	  	               relStorageHash, 
                       (void*)&fileInfoBack);
    
    if (rel == NULL)
	{
		rel = (RelData)hashMan->hashInsert(
			               hashMan,
		                   relStorageHash, 
                           (void*)&fileInfoBack);
		rel->currentBlock = INVALID_BLOCK;
	}

	return rel;
}   

/* In the moment when we are processing a buffer we 
 * want to pin it so that no other processes will
 * touch it.
 */
Bool pinBuffer(
    void*                 self,
    BufferInfo            buf, 
	BufferAccessStrategy  str)
{
	Bool res = True;
	int  i   = buf->bufInd;

    /* If ref usage count is more than 0, that means 
	 * this buffer is already pinned. In this case 
	 * we just increment the reference count.
	 */
	if (bufRefUsageCounts[i] > 0)
	{
        bufRefUsageCounts[i]++;
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
    if (str == NULL)
	{
        if (buf->usageCount < MAX_USAGE_COUNT)
            buf->usageCount++;
	}
	else
    {
		if (buf->usageCount == 0)
            buf->usageCount = 1;
	}

	bufRefUsageCounts[i]++;
	return (buf->flags & BUFFER_VALID) != 0;
}

/* The function marks buffer as available for using.
 * So that other processes can take it and 
 * start processing. */
void unpinBuffer(
	void*               self,
	BufferInfo          buf)
{
	int	  i = buf->bufInd;
    
	/* We decrement ref usage count. */
	bufRefUsageCounts[i]--;
    
	/* If after decrement of ref usage count
	 * the refcount is stille more than 0, 
	 * the buffer is still pinned.
	 */
	if (bufRefUsageCounts[i] == 0)
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

/* Finds a victim buffer in the strategy's buffer ring */
BufferInfo getBufferFromRing(
    void*                 self,
    BufferAccessStrategy  strategy)
{
	BufferInfo  buf;
	int		    bufnum;

	/* Advance to next ring slot */
	if (++strategy->current >= strategy->ring_size)
		strategy->current = 0;

	/*
	 * If the slot hasn't been filled yet, tell the caller to allocate a new
	 * buffer with the normal allocation strategy.	He will then fill this
	 * slot by calling AddBufferToRing with the new buffer.
	 */
	bufnum = strategy->buffers[strategy->current];
	if (bufnum == -1)
	{
		strategy->current_was_in_ring = False;
		return NULL;
	}

	/*
	 * If the buffer is pinned we cannot use it under any circumstances.
	 *
	 * If usage_count is 0 or 1 then the buffer is fair game (we expect 1,
	 * since our own previous usage of the ring element would have left it
	 * there, but it might've been decremented by clock sweep since then). A
	 * higher usage_count indicates someone else has touched the buffer, so we
	 * shouldn't re-use it.
	 */
	buf = &bufferInfos[bufnum - 1];
	if (buf->refCount == 0 && buf->usageCount <= 1)
	{
		strategy->current_was_in_ring = True;
		return buf;
	}

	/*
	 * Tell caller to allocate a new buffer with the normal allocation
	 * strategy.  He'll then replace this ring element via AddBufferToRing.
	 */
	strategy->current_was_in_ring = False;
	return NULL;
}

/*
 * AddBufferToRing -- add a buffer to the buffer ring
 *
 * Caller must hold the buffer header spinlock on the buffer.  Since this
 * is called with the spinlock held, it had better be quite cheap.
 */
void addBufferToRing(BufferAccessStrategy strategy, BufferInfo buf)
{
	strategy->buffers[strategy->current] = buf->bufId.blockNum + 1;
}

/* This function finds a victim buffer
 * taking into account the strategy.
 */
BufferInfo strategyGetBuffer(
	void*                 self,
	BufferAccessStrategy  strategy, 
	Bool*                 lock_held)
{
	IBufferManager   _             = (IBufferManager)self;
	ILatchManager    latchMan      = _->latchManager;

	BufferInfo       buf;
	int			     trycounter;
	Latch            bgLatch;

    if (strategy != NULL)
	{
		buf = getBufferFromRing(strategy);
		if (buf != NULL)
			return buf;
	}     

	bgLatch = StrategyControl->bgwriterLatch;
	if (bgLatch)
	{
		StrategyControl->bgwriterLatch = NULL;
		latchMan->setLatch(bgLatch);
	}

    /*
	 * Try to get a buffer from the freelist.  Note that the freeNext fields
	 * are considered to be protected by the BufFreelistLock not the
	 * individual buffer spinlocks, so it's OK to manipulate them without
	 * holding the spinlock.
	 */
	while (StrategyControl->firstFreeBuffer >= 0)
	{
		buf = &bufferInfos[StrategyControl->firstFreeBuffer];

		/* Unconditionally remove buffer from freelist */
		StrategyControl->firstFreeBuffer = buf->freeNext;
		buf->freeNext = -2; 

		/*
		 * If the buffer is pinned or has a nonzero usage_count, we cannot use
		 * it; discard it and retry.  (This can only happen if VACUUM put a
		 * valid buffer in the freelist and then someone else used it before
		 * we got to it.  It's probably impossible altogether as of 8.3, but
		 * we'd better check anyway.)
		 */
		if (buf->refCount == 0 && buf->usageCount == 0)
		{
			if (strategy != NULL)
                addBufferToRing(strategy, buf);

			return buf;
		}
	}

	/* Nothing on the freelist, so run the "clock sweep" algorithm */
	trycounter = NBuffers;
	for (;;)
	{
		buf = &bufferInfos[StrategyControl->nextVictimBuffer];

		if (++StrategyControl->nextVictimBuffer >= NBuffers)
		{
			StrategyControl->nextVictimBuffer = 0;
			StrategyControl->completePasses++;
		}

		/*
		 * If the buffer is pinned or has a nonzero usage_count, we cannot use
		 * it; decrement the usage_count (unless pinned) and keep scanning.
		 */
		if (buf->refCount == 0)
		{
			if (buf->usageCount > 0)
			{
				buf->usageCount--;
				trycounter = NBuffers;
			}
			else
			{
				/* Found a usable buffer */
				if (strategy != NULL)
					addBufferToRing(strategy, buf);
				return buf;
			}
		}
		else if (--trycounter == 0)
		{
			/*
			 * We've scanned all the buffers without making any state changes,
			 * so all the buffers are pinned (or were when we looked at them).
			 * We could hope that someone will free one eventually, but it's
			 * probably better to fail than to risk getting stuck in an
			 * infinite loop.
			 */	
		}
		
	}
}

Bool startBufferIO(BufferInfo  buf, Bool forInput)
{
    for (;;)
	{
        if (!(buf->flags & BUFFER_IO_IN_PROGRESS))
			break;
	}

	if (forInput ? (buf->flags & BUFFER_VALID) : !(buf->flags & BUFFER_DIRTY))
		return False;
	
    buf->flags |= BUFFER_IO_IN_PROGRESS;

	InProgressBuf = buf;
	IsForInput = forInput;

	return True;
}

void terminateBufferIO(
    void*            self,
    BufferInfo       buf)
{
	buf->flags &= ~(BUFFER_IO_IN_PROGRESS | BUFFER_IO_ERROR);
	if (!(buf->flags & BUFFER_JUST_DIRTIED))
		buf->flags &= ~(BUFFER_DIRTY | BUFFER_CHECKPOINT_NEEDED);
}

void flushBuffer(
		   void*          self,
		   BufferInfo     buf, 
		   RelData        rel)
{
    if (!startBufferIO(buf, False))
		return;

    /* Find smgr relation for buffer */
	if (rel == NULL)
		rel = openRelation(self, buf->bufId.relId, -1);
    
    buf->flags &= ~BUFFER_JUST_DIRTIED;

	writeBlock(
              NULL,    
			  NULL, 
			  rel,
              buf->bufId.relPart,
			  buf->bufId.blockNum,
			  (void*)(bufferBlocks + ((size_t)buf->bufInd) * BLOCK_SIZE));

    terminateBufferIO(self, buf);
}

BufferInfo allocateBuffer(
		   void*                 self,
		   RelData               rel,
		   char                  relPersist, 
		   FilePartNumber        partNum,
		   uint                  blockNum,
		   BufferAccessStrategy  strategy)
{
	IBufferManager     _          = (IBufferManager)self;
    IHashtableManager  hashMan    = _->hashtableManager;

	SBufferId	       bufId;
	BufCacheItem       bufIt;
    BufferInfo         buf;
	Bool               valid;

	bufId.relId    = rel->relKey.node;
	bufId.relPart  = partNum;
    bufId.blockNum = blockNum;

	bufIt = (BufCacheItem)hashMan->hashFind(bufCache, (void*)&bufId);

	/* When bufIt != NULL, we have found the buffer. 
	 * bufIt->id is the buffer's identifier by which we can
	 * find it in local buffer array.
	 */
	if (bufIt != NULL)
	{
		/* Get the buffer's info from the buffer array. */
		buf = &bufferInfos[bufIt->id];
        pinBuffer(buf, strategy);
		return buf;
	}

	/* We have not found the buffer in the cache.
	 * We have to find a victim buffer from the
	 */
	for (;;)
	{
		int    b;
		Bool   lock_held;
        /*
		 * Select a victim buffer.	The buffer is returned with its header
		 * spinlock still held!  Also (in most cases) the BufFreelistLock is
		 * still held, since it would be bad to hold the spinlock while
		 * possibly waking up other processes.
		 */
        bufInfo = strategyGetBuffer(_, strategy, &lock_held);
		b = bufInfo->bufId.blockNum;

		if (bufferRefUsageCounts[b] == 0)
		    bufInfo->refCount++;

	    bufferRefUsageCounts[b]++;

		if (bufInfo->flags & BUFFER_DIRTY)
		{
            if (strategy != NULL && strategyRejectBuffer(strategy, bufInfo))
		    {
                unpinBuffer(bufInfo, True);
                continue;
			}
			flushBuffer(_, bufInfo, NULL);
		}
	}
}


int readBufferExtended(
	      void*                  self,
		  char*                  execfold,
		  Relation               rel, 
		  FilePartNumber         partnum,
		  uint                   blocknum,
		  BufferAccessStrategy   strategy)
{
	IBufferManager  _      = (IBufferManager)self;
	IRelFileManager relMan = _->relFileManager; 

	BufferInfo   buf;
	BufferInfo   bufHdr;
	Bool         found;

	if (rel->data == NULL)
		openRelation(self, rel->fileId, rel->backendId);

    if (blocknum == 0xFFFFFFFF)
        blocknum = relMan->getBlocksNum(relMan, execfold, rel, partnum, REL_SEGM_SIZE);

    bufHdr = allocateBuffer(
		        self, 
				'c',
				partnum, 
				blocknum,
				strategy, 
				&found);
}

/*
 * ReadBuffer -- a shorthand for ReadBufferExtended, for reading from main
 *		fork with RBM_NORMAL mode and default strategy.
 */
//int readBuffer(Relation reln, int blockNum)
//{
//	return 0;
	//return ReadBufferExtended(reln, MAIN_FORKNUM, blockNum, RBM_NORMAL, NULL);
//}

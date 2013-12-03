
#include "relationmanager.h"

/*
const SIRelationManager sRelationManager = 
{ 
	&sHashtableManager,
	&sRelRowManager,
	createRelation,
	createRelationCache
};

const IRelationManager relationManager = &sRelationManager;
*/

Hashtable    relStorageHash = NULL;
Hashtable    sharedBufHash = NULL;
BufferInfo   bufferInfos;
int*         bufferRefUsageCounts;
int			 NBuffers = 1000;

/* Pointers to shared state */
BufferStrategyControl StrategyControl = NULL;

/*
 * ReadBuffer -- a shorthand for ReadBufferExtended, for reading from main
 *		fork with RBM_NORMAL mode and default strategy.
 */
int readBuffer(Relation reln, int blockNum)
{
	return ReadBufferExtended(reln, MAIN_FORKNUM, blockNum, RBM_NORMAL, NULL);
}

void SetLatch(Latch latch)
{
	HANDLE		handle;

	/* Quick exit if already set */
	if (latch->is_set)
		return;

	latch->is_set = true;

	/*
	 * See if anyone's waiting for the latch. It can be the current process if
	 * we're in a signal handler.
	 *
	 * Use a local variable here just in case somebody changes the event field
	 * concurrently (which really should not happen).
	 */
	handle = latch->event;
	if (handle)
	{
		SetEvent(handle);

		/*
		 * Note that we silently ignore any errors. We might be in a signal
		 * handler or other critical path where it's not safe to call elog().
		 */
	}
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
bool strategyRejectBuffer(BufferAccessStrategy strategy, BufferDesc buf)
{
	/* We only do this in bulkread mode */
	if (strategy->btype != BAS_BULKREAD)
		return false;

	/* Don't muck with behavior of normal buffer-replacement strategy */
	if (!strategy->current_was_in_ring ||
	     strategy->buffers[strategy->current] 
	     != buf->buf_id + 1)
		return false;

	/*
	 * Remove the dirty buffer from the ring; necessary to prevent infinite
	 * loop if all ring members are dirty.
	 */
	strategy->buffers[strategy->current] = -1;

	return true;
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
		                   relStorageHash, 
                           (void*)&fileInfoBack);
		rel->currentBlock = INVALID_BLOCK;
	}

	return rel;
}   

Bool pinBuffer(BufferInfo bufInfo)
{
	int i = bufInfo->bufId;

    if (bufferRefUsageCounts[i] == 0)
	{
		bufInfo->refcount++;
        if (bufInfo->usageCount < MAX_USAGE_COUNT)
            bufInfo->usageCount++;

		bufInfo->flags & 
		return (buf->flags & BUFFER_VALID) != 0;
	}
    bufferRefUsageCounts[i]++;
    return True;
}

/*
 * UnpinBuffer -- make buffer available for replacement.
 *
 * This should be applied only to shared buffers, never local ones.
 *
 * Most but not all callers want CurrentResourceOwner to be adjusted.
 * Those that don't should pass fixOwner = FALSE.
 */
void unpinBuffer(BufferDesc buf, bool fixOwner)
{
	int			b = buf->buf_id;

	Assert(PrivateRefCount[b] > 0);
	PrivateRefCount[b]--;
	if (PrivateRefCount[b] == 0)
	{
		/* I'd better not still hold any locks on the buffer */
		Assert(!LWLockHeldByMe(buf->content_lock));
		Assert(!LWLockHeldByMe(buf->io_in_progress_lock));

		LockBufHdr(buf);

		/* Decrement the shared reference count */
		Assert(buf->refcount > 0);
		buf->refcount--;

		/* Support LockBufferForCleanup() */
		if ((buf->flags & BM_PIN_COUNT_WAITER) &&
			buf->refcount == 1)
		{
			/* we just released the last pin other than the waiter's */
			int			wait_backend_pid = buf->wait_backend_pid;

			buf->flags &= ~BM_PIN_COUNT_WAITER;
			UnlockBufHdr(buf);
			ProcSendSignal(wait_backend_pid);
		}
		else
			UnlockBufHdr(buf);
	}
}

BufferInfo getBufferFromRing(BufferAccessStrategy strategy)
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
	if (buf->refcount == 0 && buf->usage_count <= 1)
	{
		strategy->current_was_in_ring = true;
		return buf;
	}

	/*
	 * Tell caller to allocate a new buffer with the normal allocation
	 * strategy.  He'll then replace this ring element via AddBufferToRing.
	 */
	strategy->current_was_in_ring = false;
	return NULL;
}

/*
 * AddBufferToRing -- add a buffer to the buffer ring
 *
 * Caller must hold the buffer header spinlock on the buffer.  Since this
 * is called with the spinlock held, it had better be quite cheap.
 */
void addBufferToRing(BufferAccessStrategy strategy, BufferDesc buf)
{
	strategy->buffers[strategy->current] = buf->buf_id + 1;
}

BufferInfo strategyGetBuffer(BufferAccessStrategy strategy, Bool* lock_held)
{
	BufferDesc   buf;
	int			 trycounter;

    if (strategy != NULL)
	{
		buf = getBufferFromRing(strategy);
		if (buf != NULL)
			return buf;
	}     

	bgwriterLatch = StrategyControl->bgwriterLatch;
	if (bgwriterLatch)
	{
		StrategyControl->bgwriterLatch = NULL;
		SetLatch(bgwriterLatch);
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
		if (buf->refcount == 0 && buf->usage_count == 0)
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
		if (buf->refcount == 0)
		{
			if (buf->usage_count > 0)
			{
				buf->usage_count--;
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


BufferInfo allocateBuffer(
		   void*             self,
		   RelData           rel,
		   char              relPersist, 
		   FilePartNumber    partNum,
		   uint32            blockNum)
{
	IBufferManager     _          = (IBufferManager)self;
    IHashtableManager  hashMan    = _->hashtableManager;

	SBufferId	       bufId;
	BufCacheItem       bufCache;
    BufferInfo         bufInfo;

	bufId->relId    = rel->relKey.node;
	bufId->relPart  = partNum;
    bufId->blockNum = blockNum;

	bufCache = (BufCacheItem)hashMan->hashFind(sharedBufHash, (void*)&bufId);

	if (bufCache)
	{
		bufInfo = &bufferInfos[bufCache->id];
        valid = PinBuffer(buf, strategy);
		if (!valid)
		{
            /* Here should be a code in case 
			 * when other processes are holding a pin */
		}
		return bufInfo;
	}

	/*
	 * Didn't find it in the buffer pool.  We'll have to initialize a new
	 * buffer.	Remember to unlock the mapping lock while doing the work.
	 */
    /* Loop here in case we have to try another victim buffer */
	for (;;)
	{
		bool		lock_held;
        /*
		 * Select a victim buffer.	The buffer is returned with its header
		 * spinlock still held!  Also (in most cases) the BufFreelistLock is
		 * still held, since it would be bad to hold the spinlock while
		 * possibly waking up other processes.
		 */
        buf = StrategyGetBuffer(strategy, &lock_held);
		int			b = buf->buf_id;

		if (bufferRefUsageCounts[buf->buf_id] == 0)
		    buf->refcount++;

	    bufferRefUsageCounts[buf->buf_id]++;

		if (buf->flags & BM_DIRTY)
		{
            if (strategy != NULL && strategyRejectBuffer(strategy, buf))
		    {
                
			}
		}
	}
}

/*
 * BufTableHashCode
 *		Compute the hash code associated with a BufferTag
 *
 * This must be passed to the lookup/insert/delete routines along with the
 * tag.  We do it like this because the callers need to know the hash code
 * in order to determine which buffer partition to lock, and we don't want
 * to do the hash computation twice (hash_any is a bit slow).
 */
uint32
BufTableHashCode(BufferTag *tagPtr)
{
	return get_hash_value(SharedBufHash, (void *) tagPtr);
}


}

int readBufferExtended(
	      void*                  self,
		  Relation               rel, 
		  FilePartNumber         partnum,
		  uint32                 blocknum)
{
	IBufferManager  _      = (IBufferManager)self;
	IRelFileManager relMan = _->relFileManager; 

	if (rel->data == NULL)
		openRelation(self, rel->fileId, rel->backendId);

	buf = ReadBuffer_common(
		      reln->rd_smgr, 
			  reln->rd_rel->relpersistence,
			  forkNum, 
			  blockNum, 
			  mode, 
			  strategy, 
			  &hit);

    if (blockNum == 0xFFFFFFFF)
        blockNum = relMan->getBlocksNum(relMan, rel, partnum);

    bufHdr = BufferAlloc(smgr, relpersistence, forkNum, blockNum,
							 strategy, &found);



}




#include "buffermanager.h"
#include "latch.h"
#include "latchmanager.h"
#include "imemorymanager.h"

const SIBufferManager sBufferManager = 
{ 
	&sHashtableManager,
	&sRelRowManager,
    &sLatchManager
};

const IBufferManager bufferManager = &sBufferManager;

Hashtable    relStorageHash = NULL;

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
BufFreeListState  freeBufferState   = NULL;

/* local state for StartBufferIO and related functions */
BufferInfo        InProgressBuf = NULL;
Bool              IsForInput;


void initFreeBufferState(void* self)
{
    IBufferManager      _    = (IBufferManager)self;
	IHashtableManager   hm   = _->hashtableManager;
	IMemoryManager      mm   = hm->memManager;

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
	IMemoryManager      mm   = hm->memManager;

	rfm->ctorRelFileMan(rfm);
    
	bufInfos  = (BufferInfo)mm->alloc(bufNum * sizeof(SBufferInfo));
	bufBlocks = (char*)mm->alloc(bufNum * BLOCK_SIZE);

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

/* This function finds a victim buffer
 * taking into account the strategy.
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

    /* Try to get a buffer from the free list */
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
	 * "clockwise algorithm"
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
	IBufferManager     _          = (IBufferManager)self;
	IRelFileManager    rfm         = _->relFileManager;

    if (!startBufferIO(buf, False))
		return;

    /* Find smgr relation for buffer */
	if (rel == NULL)
		rel = NULL; //openRelation(self, buf->bufId.relId, -1);

    buf->flags &= ~BUFFER_JUST_DIRTIED;

	rfm->writeBlock(
              NULL,
			  NULL,
			  rel,
              buf->id.relPart,
			  buf->id.blockNum,
			  (void*)(bufBlocks + ((size_t)buf->ind) * BLOCK_SIZE));

    terminateBufferIO(self, buf);
}

BufferInfo allocateBuffer(
		   void*                 self,
		   RelData               rel,
		   char                  relPersist, 
		   FilePartNumber        partNum,
		   uint                  blockNum,
		   BufRing               ring)
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
		buf = &bufInfos[bufIt->id];
		_->pinBuffer(_, buf, ring);
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
        buf = getBufferFromRing(_, ring);
		b   = buf->id.blockNum;

		if (bufPrivateRefUsageCounts[b] == 0)
		    buf->refCount++;

	    bufPrivateRefUsageCounts[b]++;

		if (buf->flags & BUFFER_DIRTY)
		{
            if (ring != NULL && strategyRejectBuffer(ring, buf))
		    {
                unpinBuffer(buf, True);
                continue;
			}
			flushBuffer(_, buf, NULL);
		}
	}
}


int readBufferExtended(
	      void*                  self,
		  char*                  execfold,
		  Relation               rel, 
		  FilePartNumber         partnum,
		  uint                   blocknum,
		  BufRing                ring)
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
				ring, 
				&found);
}



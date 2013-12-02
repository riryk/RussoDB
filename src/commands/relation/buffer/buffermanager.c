
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

/*
 * ReadBuffer -- a shorthand for ReadBufferExtended, for reading from main
 *		fork with RBM_NORMAL mode and default strategy.
 */
int readBuffer(Relation reln, int blockNum)
{
	return ReadBufferExtended(reln, MAIN_FORKNUM, blockNum, RBM_NORMAL, NULL);
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

BufferInfo strategyGetBuffer(BufferAccessStrategy strategy, Bool* lock_held)
{
   
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
    /*
		 * Select a victim buffer.	The buffer is returned with its header
		 * spinlock still held!  Also (in most cases) the BufFreelistLock is
		 * still held, since it would be bad to hold the spinlock while
		 * possibly waking up other processes.
		 */
    buf = StrategyGetBuffer(strategy, &lock_held);
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



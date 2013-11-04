
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

Hashtable relStorageHash = NULL;

/*
 * ReadBuffer -- a shorthand for ReadBufferExtended, for reading from main
 *		fork with RBM_NORMAL mode and default strategy.
 */
int readBuffer(Relation reln, int blockNum)
{
	return ReadBufferExtended(reln, MAIN_FORKNUM, blockNum, RBM_NORMAL, NULL);
}

/*
 * RelationOpenSmgr
 *		Open the relation at the smgr level, if not already done.
 */
#define RelationOpenSmgr(relation) \
	do { \
		if ((relation)->rd_smgr == NULL) \
			smgrsetowner(&((relation)->rd_smgr), \
                         smgropen((relation)->rd_node, \
                         (relation)->rd_backend)); \
	} while (0)



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

/*
 *	mdnblocks() -- Get the number of blocks stored in a relation.
 *
 *		Important side effect: all active segments of the relation are opened
 *		and added to the mdfd_chain list.  If this routine has not been
 *		called, then only segments up to the last one actually touched
 *		are present in the chain.
 */
BlockNumber
mdnblocks(SMgrRelation reln, ForkNumber forknum)
{

}

int readBufferExtended(
	      void*                  self,
		  Relation               rel, 
		  ForkNumber             forkNum, 
		  BlockNumber            blockNum,
		  ReadBufferMode         mode, 
		  BufferAccessStrategy   strategy)
{
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

    /*
	 *	smgrnblocks() -- Calculate the number of blocks in the
	 *					 supplied relation.
	 */
	BlockNumber
	smgrnblocks(SMgrRelation reln, ForkNumber forknum)
	{
		return (*(smgrsw[reln->smgr_which].smgr_nblocks)) (reln, forknum);
	}

    if (blockNum == P_NEW)
    {
       
    }
}



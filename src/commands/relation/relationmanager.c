
#include "relationmanager.h"

const SIRelationManager sRelationManager = 
{ 
	&sHashtableManager,
	&sRelRowManager,
	createRelation,
	createRelationCache,
    openRelation
};

const IRelationManager relationManager = &sRelationManager;

Hashtable    relCache       = NULL;
Hashtable    relStorageHash = NULL;

void createRelation(
	void*            self,
	char*            relName, 
	uint             relTypeId, 
	uint             attrCount, 
	RelAttribute     attributes)
{
    IHashtableManager _ = (IHashtableManager)self;

	int               found;
	RelCacheItem      relItem;
    Relation          rel = (Relation)malloc(sizeof(SRelation));

	rel->refCount   = 1;
	rel->typeId     = relTypeId;
	rel->attrCount  = attrCount;
	rel->attributes = attributes;

	relItem = (RelCacheItem)_->hashInsert(_, relCache, (void*)&(rel->id));
	relItem->relation = rel;
}

void createRelationCache(void* self)
{
    IHashtableManager _ = (IHashtableManager)self;
	SHashtableSettings sett;

	memset(&sett, 0, sizeof(sett));

	sett.keyLen = sizeof(uint);
	sett.valLen = sizeof(SRelCacheItem) - sizeof(uint);
	sett.hashFunc = getHashId;

	relCache = _->createHashtable(_,
		                          "Relation cache",
								  400,
								  &sett,
								  HASH_FUNC);
}

/* Gets a buffer with free space more than len */
int getBufferForRelRow(
	void*           self,
	Relation        rel,
	size_t          len)
{
	IRelationManager   _  = (IRelationManager)self;
	IBufferManager     bm = (IBufferManager)_->bufferManager;
	IPageManager       pm = (IPageManager)_->pageManager;

    size_t     extraFreeSpace;
    size_t     pageFreeSpace;
    Bool	   useFreeStorageManager = False;
	int        buffer;
	void*      page;
	int        blockNum;

    /* We should not fill in our page more than 'fillPercent' percent.
     * This macros calculates how much space should be left freed. 
	 */
    extraFreeSpace = GetPageFreeSpace(rel, REL_FILL_PERCENT_DEFAULT); 
    blockNum       = RelGetCurrentBlock(rel);

    if (len + extraFreeSpace > MAX_ROW_SIZE)
    {
	    blockNum = INVALID_BLOCK;
	    useFreeStorageManager = False;
    }
    
	if (blockNum == INVALID_BLOCK)
	{
		int nblocks = bm->getBlocksNumInRelPart(bm, rel->data, FILE_PART_MAIN);

	    if (nblocks > 0)
		    blockNum = nblocks - 1;
	}

    if (blockNum != INVALID_BLOCK)
    {
		buffer = bm->readBuffer(bm, rel->data, FILE_PART_MAIN, blockNum, NULL);

		/* Here we should check if there is free. 
		 * If it is, we return the current buffer.
		 */
		page = (void*)(bufBlocks + ((size_t)(buffer - 1)) * BlockSize);

		pageFreeSpace = pm->getFreeSpace(pm, page);;
		if (len + extraFreeSpace <= pageFreeSpace)
		{
			rel->data->currentBlock = blockNum;
			return buffer;
		}
	}

	/* We have not found a buffer. Try to extend a table 
	 * If we put BUFFER_NEW as bufferNum parameter, we will extend the relation.
	 */
    buffer = bm->readBuffer(bm, rel->data, FILE_PART_MAIN, BUFFER_NEW, NULL);
    page   = (void*)(bufBlocks + ((size_t)(buffer - 1)) * BlockSize);

	/* Initialize a new page */
    pm->initializePage(pm, page, BlockSize, 0);

	rel->data->currentBlock = bm->getBlockNum(buffer);
	return buffer;
}

void addRowToBuffer(
    void*           self,
	Relation        rel,
	int             buf,
	RelRow          row)
{
    IRelationManager _  = (IRelationManager)self;
    IPageManager     pm = (IPageManager)_->pageManager;
	IBufferManager   bm = (IBufferManager)_->bufferManager;

    int              blocknum;
	ItemPointer           itemId;
	void*            item;

    void*     pageHdr = (void*)(bufBlocks + ((size_t)(buf - 1)) * BlockSize);
    uint16    offnum  = pm->addItemToPage(
	                       pageHdr,
						   (void*)row->data,
						   row->len,
	                       0,
                           False);

    if (offnum == 0)
		return;

	blocknum = bm->getBlockNum(buf);

	SetBlockIdTo(&(row->self.block), blocknum);
	row->self.pos = offnum;

	itemId = PageGetItemId(pageHdr, offnum);
	item   = (char*)(pageHdr) + itemId->off;

	((RelRowHeader)item)->curr = row->self;
}

RelRow beforeInsert(
    void*           self,
	RelRow          row,
	int             relAttrsCount,
	RelAttribute    relAttrs,
	uint            tranId,
	uint            cmdId)
{
	IRelationManager _  = (IRelationManager)self;
	IRelRowManager   rm = _->relRowManager;

	RelRowHeader     rowData = row->data; 
	Bool             isExternal;

    rowData->mask  &= ~(ROW_TRAN_MASK);
    rowData->mask2 &= ~(ROW_TRAN_MASK2);
    rowData->mask  |= ROW_TRAN_MIN_INVALID;

	RelRowSetCmdId(row, cmdId);
	RelRowSetTranMin(row, tranId);
    RelRowSetTranMax(row, 0);

	isExternal = HeapTupleHasExternal(row);

	if (isExternal || row->len > MAX_ROW_SIZE)
    {
       row = rm->shortenRow(row, 
							NULL,
				            relAttrsCount,
			                relAttrs);
	}

	return row;
}

void insertIntoTable(void*           self,
					 Relation        rel,
					 int             relAttrsCount,
			         RelAttribute    relAttrs,
					 RelRow          row,
					 uint            cmdId)
{
	IRelationManager   _ = (IRelationManager)self;
    IBufferManager     bm = (IBufferManager)_->bufferManager;
    int                buf;

	row = _->beforeInsert(_,
	                 row,
	                 relAttrsCount,
	                 relAttrs,
	                 0,
	                 cmdId);

	buf = _->getBufferForRelRow(_, rel, row->len);

    _->addRowToBuffer(_, rel, buf, row);
	bm->dirtyBuffer(buf);
}

RelData openRelation(
    void*          self,
	RelFileInfo    fileInfo, 
	int            backend)
{
	IRelationManager   _             = (IRelationManager)self;
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

	fileInfoBack.node    = *fileInfo; 
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

/*#define RelationRelation_Rowtype_Id  83

SRelAttribute Desc_pg_class[Natts_pg_class] = {catalog_relation};

formrdesc("pg_class", 
		  RelationRelation_Rowtype_Id, 
		  false,
		  true, 
		  Natts_pg_class, 
		  Desc_pg_class);*/


#include "relationmanager.h"

const SIRelationManager sRelationManager = 
{ 
	&sHashtableManager,
	&sRelRowManager,
	createRelation,
	createRelationCache
};

const IRelationManager relationManager = &sRelationManager;

Hashtable relCache;

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

void InsertIntoTable(void*           self,
					 Relation        rel,
					 int             relAttrsCount,
			         RelAttribute    relAttrs,
					 RelRow          row)
{
   IRelationManager _ = (IRelationManager)self;
   //Relation	boot_reldesc;		/* current relation descriptor */

   //simple_heap_insert(boot_reldesc, tuple);
   //heap_insert(relation, tup, GetCurrentCommandId(true), 0, NULL);

   //heaptup = heap_prepare_insert(relation, tup, xid, cid, options); 
   int        buffer;
   uint       tranId;
   uint       cmdId;
   Bool       isExternal;
   size_t     extraFreeSpace;
   int        blockNum;
   int        otherBlockNum = INVALID_BLOCK;
   Bool		  useFreeStorageManager = False;

   row->data->mask  &= ~(ROW_TRAN_MASK);
   row->data->mask2 &= ~(ROW_TRAN_MASK2);
   row->data->mask  |= ROW_TRAN_MIN_INVALID;

   row->data->typeData.fields.tranMin = tranId;
   row->data->typeData.fields.tranMax = 0;
   row->data->typeData.fields.field3.cmdId = cmdId;

   isExternal = row->data->mask & ROW_HASEXTERNAL != 0;
   if (isExternal || row->len > MaxRowSize_By_RowsPerPage(ROWS_PER_PAGE)  /*MAX_ROW_SIZE*/)
   {
	   row = _->relRowManager->shortenRow(
	                        row, 
							NULL,
				            relAttrsCount,
			                relAttrs);
   }

   /* We should not fill in our page more than 'fillPercent' percent.
    * This macros calculates how musch space should be left freed. 
	*/
   extraFreeSpace = GetPageFreeSpace(rel, REL_FILL_PERCENT_DEFAULT); 

   if (row->len + extraFreeSpace > MAX_ROW_SIZE)
   {
	   blockNum = INVALID_BLOCK;
	   useFreeStorageManager = True;
   }
   else
	   blockNum = RelGetCurrentBlock(rel);
   
   while (blockNum != INVALID_BLOCK)
   {  
	   if (otherBlockNum == INVALID_BLOCK)
	   {
		   /* easy case */
		   // buffer = ReadBufferBI(relation, targetBlock, bistate);
		   // ReadBuffer(relation, targetBlock);
		   buffer = ReadBufferBI(relation, targetBlock, bistate);
	   } 
   }
}



void readBuffer(
    void*           self,
	char*           fold,
    Relation        rel,
	int             block,
	FilePartNumber  part)
{ 
    IRelationManager _  = (IRelationManager)self;
 	IRelRowManager   rm = _->relRowManager;
	IRelFileManager  rf = _->relFileManager;
    
    if (block == -1)
		block = rf->getBlocksNum(
		              rf, 
		              fold, 
					  rel->data, 
					  part, 
					  REL_SEGM_SIZE);
    
}

/*#define RelationRelation_Rowtype_Id  83

SRelAttribute Desc_pg_class[Natts_pg_class] = {catalog_relation};

formrdesc("pg_class", 
		  RelationRelation_Rowtype_Id, 
		  false,
		  true, 
		  Natts_pg_class, 
		  Desc_pg_class);*/

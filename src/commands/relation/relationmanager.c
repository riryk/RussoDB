
#include "relationmanager.h"

const SIRelationManager sRelationManager = 
{ 
	&sHashtableManager,
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

/*#define RelationRelation_Rowtype_Id  83

SRelAttribute Desc_pg_class[Natts_pg_class] = {catalog_relation};

formrdesc("pg_class", 
		  RelationRelation_Rowtype_Id, 
		  false,
		  true, 
		  Natts_pg_class, 
		  Desc_pg_class);*/

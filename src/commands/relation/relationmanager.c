
#include "relationmanager.h"


void createRelation(
	char* relName, 
	uint relTypeId, 
	uint attrCount, 
	SRelAttribute* attributes)
{
    Relation relation = (Relation)malloc(sizeof(SRelation));

	relation->refCount = 1;
	relation->typeId = relTypeId;
	relation->attrCount = attrCount;
	relation->attributes = attributes;

}

void createRelationCache()
{
	SHashtableSettings sett;

	memset(&sett, 0, sizeof(sett));

	sett.keyLen = sizeof(uint);
	sett.valLen = sizeof(SRelCacheItem);
	sett.hashFunc = hashFuncForIds;

	RelationCache = createHashtable("Relation cache",
									400,
									&sett,
								    HASH_FUNC);
}

#include "relationmanager.h"


void createRelation(
	char* relName, 
	uint relTypeId, 
	uint attrCount, 
	SRelAttribute* attributes)
{
	int found;
    Relation relation = (Relation)malloc(sizeof(SRelation));

	relation->refCount = 1;
	relation->typeId = relTypeId;
	relation->attrCount = attrCount;
	relation->attributes = attributes;

	/*hash_search(RelationCache, 
		        (void*)&(relation->id), 
				HASH_INSERT, 
				&found); */
}

void createRelationCache()
{
	SHashtableSettings sett;

	memset(&sett, 0, sizeof(sett));

	sett.keyLen = sizeof(uint);
	sett.valLen = sizeof(SRelCacheItem);
	sett.hashFunc = getHashId;

	RelationCache = createHashtable("Relation cache",
									400,
									&sett,
								    HASH_FUNC);
}
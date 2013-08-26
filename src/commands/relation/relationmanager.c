
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
	struct HashTableSettings* sett;

	memset(&sett, 0, sizeof(sett));

	sett->KeyLength = sizeof(uint);
    sett->ValueLength = sizeof(SRelCacheItem);
    sett->HashFunc = HashForRelId;

	RelationCache = HashTableCreate("Relation cache",
									400,
									&sett,
								    HASH_FUNCTION);
}
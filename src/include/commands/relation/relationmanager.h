
#ifndef RELATION_MANAGER_H
#define RELATION_MANAGER_H

#include "hashtablemanager.h"
#include "rel.h"
#include "irelationmanager.h"

Hashtable RelationCache;

void createRelation(
	char*          relName, 
	uint           relTypeId, 
	uint           attrCount, 
	RelAttribute   attributes);

void createRelationCache();

#endif
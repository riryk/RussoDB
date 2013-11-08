
#ifndef RELATION_MANAGER_H
#define RELATION_MANAGER_H

#include "hashtablemanager.h"
#include "rel.h"
#include "irelationmanager.h"
#include "relrowmanager.h"

Hashtable RelationCache;

void createRelation(
	void*          self,
	char*          relName, 
	uint           relTypeId, 
	uint           attrCount, 
	RelAttribute   attributes);

void createRelationCache(void* self);

#endif

#ifndef RELATION_MANAGER_H
#define RELATION_MANAGER_H

#include "hashtablemanager.h"
#include "rel.h"
#include "irelationmanager.h"
#include "relrowmanager.h"
#include "relfilemanager.h"
#include "irelfilemanager.h"
#include "ibuffermanager.h"
#include "buffermanager.h"

void createRelation(
	void*          self,
	char*          relName, 
	uint           relTypeId, 
	uint           attrCount, 
	RelAttribute   attributes);

void createRelationCache(void* self);

RelData openRelation(
    void*          self,
	RelFileInfo    fileInfo, 
	int            backend);

#endif

#ifndef IRELATION_MANAGER_H
#define IRELATION_MANAGER_H

#include "common.h"
#include "hashtable.h"

typedef struct SIRelationManager
{   
	IHashtableManager hashtableManager;

	void (*createRelation)(
	     char*          relName, 
	     uint           relTypeId, 
	     uint           attrCount, 
	     SRelAttribute* attributes);

    void createRelationCache();

} SIRelationManager, *IRelationManager;


#endif
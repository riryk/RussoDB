
#ifndef IRELATION_MANAGER_H
#define IRELATION_MANAGER_H

#include "hashtable.h"
#include "rel.h"

typedef struct SIRelationManager
{   
	IHashtableManager hashtableManager;

	void (*createRelation)(
	     char*          relName, 
	     uint           relTypeId, 
	     uint           attrCount, 
	     RelAttribute   attributes);

    void createRelationCache();

} SIRelationManager, *IRelationManager;


#endif
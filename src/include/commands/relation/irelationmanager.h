
#ifndef IRELATION_MANAGER_H
#define IRELATION_MANAGER_H

#include "hashtable.h"
#include "rel.h"

typedef struct SIRelationManager
{   
	IHashtableManager hashtableManager;
	IRelRowManager    relRowManager;

	void (*createRelation)(
	     void*            self,
	     char*            relName, 
	     uint             relTypeId, 
	     uint             attrCount, 
	     RelAttribute     attributes);

	void (*createRelationCache)(void* self);

} SIRelationManager, *IRelationManager;


#endif
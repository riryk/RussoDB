
#ifndef IRELATION_MANAGER_H
#define IRELATION_MANAGER_H

#include "hashtable.h"
#include "rel.h"
#include "ihashtablemanager.h"
#include "irelrowmanager.h"
#include "irelfilemanager.h"
#include "ibuffermanager.h"
#include "ipagemanager.h"

typedef struct SIRelationManager
{   
	IHashtableManager hashtableManager;
	IRelRowManager    relRowManager;
	IRelFileManager   relFileManager;
    IBufferManager    bufferManager;
    IPageManager      pageManager;

	void (*createRelation)(
	     void*            self,
	     char*            relName, 
	     uint             relTypeId, 
	     uint             attrCount, 
	     RelAttribute     attributes);

	void (*createRelationCache)(void* self);

	RelData (*openRelation)(
         void*            self,
	     RelFileInfo      fileInfo, 
	     int              backend);

} SIRelationManager, *IRelationManager;


#endif
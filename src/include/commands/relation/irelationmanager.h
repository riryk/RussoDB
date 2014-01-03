
#ifndef IRELATION_MANAGER_H
#define IRELATION_MANAGER_H

#include "hashtable.h"
#include "rel.h"
#include "ihashtablemanager.h"
#include "irelrowmanager.h"
#include "irelfilemanager.h"
#include "ipagemanager.h"

typedef struct SIRelationManager
{   
	IHashtableManager hashtableManager;
	IRelRowManager    relRowManager;
	IRelFileManager   relFileManager;
    void*             bufferManager;
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
    
	RelRow (*beforeInsert)(
         void*           self,
	     RelRow          row,
	     int             relAttrsCount,
	     RelAttribute    relAttrs,
	     uint            tranId,
	     uint            cmdId);

	int (*getBufferForRelRow)(
	     void*           self,
	     Relation        rel,
	     size_t          len);
    
	void (*addRowToBuffer)(
         void*           self,
	     Relation        rel,
	     int             buf,
	     RelRow          row);

} SIRelationManager, *IRelationManager;


#endif
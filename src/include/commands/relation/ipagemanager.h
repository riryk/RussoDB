
#ifndef IPAGE_MANAGER_H
#define IPAGE_MANAGER_H

#include "hashtable.h"
#include "rel.h"
#include "ihashtablemanager.h"

typedef struct SIPageManager
{   
	IHashtableManager hashtableManager;

	void (*initializePage)(
         void*            self,
	     void*            page, 
	     size_t           size, 
	     size_t           suplSize);

	size_t (*getFreeSpace)(
	     void*            self,
	     void*            page);
    
	uint16 (*addItemToPage)(
	     void*            page,
         void*            item,
         size_t           size,
	     uint16           itemnum,
         Bool             overwrite);

} SIPageManager, *IPageManager;

#endif
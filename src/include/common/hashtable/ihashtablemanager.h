#include "hashtable.h"
#include "imemorymanager.h"

#ifndef IHASHTABLE_MANAGER_H
#define IHASHTABLE_MANAGER_H


typedef struct SIHashtableManager
{
	IMemoryManager memManager;

    Hashtable (*createHashtable)(
		void*              self,
		char*              name, 
		long               maxItemsNum, 
		HashtableSettings  set, 
		int                setFlags);
     
    void* (*hashFind)(
	    Hashtable          tbl, 
        void*              key);

    void* (*hashInsert)(
		void*              self,
        Hashtable          tbl, 
        void*              key);

} SIHashtableManager, *IHashtableManager;


#endif
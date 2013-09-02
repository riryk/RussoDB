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
} SIHashtableManager, *IHashtableManager;


#endif
#include "hashtable.h"
#include "imemorymanager.h"
#include "hashtablehelper.h"
#include "ierrorlogger.h"
#include "ispinlockmanager.h"

#ifndef IHASHTABLE_MANAGER_H
#define IHASHTABLE_MANAGER_H


typedef struct SIHashtableManager
{
	IMemoryManager    memManager;
	ICommon           commonHelper;
	IHashtableHelper  hashtableHelper;
	ISpinLockManager  spinLockHelper; 

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

	void* (*hashRemove)(
	    void*              self,
        Hashtable          tbl, 
        void*              key);

	IErrorLogger      errorLogger;

} SIHashtableManager, *IHashtableManager;


#endif
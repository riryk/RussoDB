#include "ihashtablemanager.h"
#include "trackmemmanager.h"

#ifndef HASHTABLE_MANAGER_H
#define HASHTABLE_MANAGER_H

extern const IHashtableManager hashtableManager;

uint itemsNumToAlloc(uint elemSize);

Hashtable createHashtable(
    void*              self,
	char*              name, 
	long               maxItemsNum, 
	HashtableSettings  set, 
	int                setFlags);

void hashLookUp(
    void*              self,
    Hashtable          tbl, 
    void*              key,
    EHashAction        act);

#endif
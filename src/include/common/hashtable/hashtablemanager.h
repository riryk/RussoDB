#include "ihashtablemanager.h"
#include "trackmemmanager.h"
#include "common.h"

#ifndef HASHTABLE_MANAGER_H
#define HASHTABLE_MANAGER_H

extern const SIHashtableManager sHashtableManager;
extern const IHashtableManager  hashtableManager;


#define HASH_ELEM_SIZE(tbl) \
( \
    AlignDefault(sizeof(SHashItem)) \
  + AlignDefault((tbl)->keyLen) \
  + AlignDefault((tbl)->valLen) \
)

uint itemsNumToAlloc(uint elemSize);

Hashtable createHashtable(
    void*              self,
	char*              name, 
	long               maxItemsNum, 
	HashtableSettings  set, 
	int                setFlags);

void* hashFind(
	Hashtable          tbl, 
    void*              key);

void* hashInsert(
	void*              self,
    Hashtable          tbl, 
    void*              key);

void* hashRemove(
	void*              self,
    Hashtable          tbl, 
    void*              key);

#endif
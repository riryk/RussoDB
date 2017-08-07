#include "ihashtablemanager.h"
#include "trackmemmanager.h"
#include "common.h"

#ifndef HASHTABLE_MANAGER_H
#define HASHTABLE_MANAGER_H

extern const SIHashtableManager sHashtableManager;
extern const IHashtableManager  hashtableManager;

typedef struct
{
   Hashtable* hashTable;
   int currentBucket;
   HashItem* currentEntry; 
}  HashSequentialScanStatus;

#define HASH_ELEM_SIZE(tbl) \
( \
    AlignDefault(sizeof(SHashItem)) \
  + AlignDefault((tbl)->keyLen) \
  + AlignDefault((tbl)->valLen) \
)

uint itemsNumToAlloc(uint elemSize);

Hashtable createHashtable(
	char*              name, 
	long               maxItemsNum, 
	HashtableSettings  set, 
	int                setFlags);

void* hashFind(
	Hashtable          tbl, 
  void*              key);

void* hashInsert(
  Hashtable tbl, 
  void* key,
  Bool* wasFoundBeforeInsert);

void* hashRemove(
  Hashtable          tbl, 
  void*              key);

void hashSequentialScanInit(HashSequentialScanStatus* status, Hashtable* hashTable);

void* hashSequentialSearch(HashSequentialScanStatus* status);

void hashSequentialSearchTerminate(HashSequentialScanStatus* status);

#endif
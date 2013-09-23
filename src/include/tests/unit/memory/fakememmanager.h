
#include "imemorymanager.h"

#ifndef FAKE_MEMORY_MANAGER_H
#define FAKE_MEMORY_MANAGER_H

typedef struct SMemHistoryItem
{
	void*            mem; 
	uint	         memLen;  
} SMemHistoryItem, *MemHistoryItem;

extern const SIMemoryManager sFakeMemManager;
extern const IMemoryManager fakeMemManager;

extern SMemHistoryItem fakeMemStorage[10000];
extern uint fakeMemStorageCount;

#endif



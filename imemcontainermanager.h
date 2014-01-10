#include "common.h"
#include "memory.h"

#ifndef IMEM_CONTAINER_MANAGER_H
#define IMEM_CONTAINER_MANAGER_H

typedef struct SIMemContainerManager
{
    void* (*alloc)(uint size);	
	void* (*realloc)(void* newMem, uint size);
	void (*free)(void* mem);
	void (*freeAll)();
    
} SIMemContainerManager, *IMemContainerManager;

#endif

#ifndef IMEM_CONTAINER_MANAGER_H
#define IMEM_CONTAINER_MANAGER_H

#include "common.h"
#include "memory.h"

typedef struct SIMemContainerManager
{
	void*  errorLogger;

	void (*ctorMemContMan)(
         void*                self, 
	     FMalloc              funcMallocParam,
	     FFree                funcFreeParam);

	void (*resetErrCont)(void* self);

	MemoryContainer (*changeToErrorContainer)();

	void (*dtorMemContMan)(
		 void*                self);

	MemoryContainer (*memContCreate)(
	     void*                self,
         MemoryContainer      container,
	     MemoryContainer      parent,
         MemContType          type, 
	     size_t               size,
	     char*                name);

	MemorySet (*memSetCreate)(
         void*                self,
	     MemoryContainer      container,
         MemoryContainer      parent,
         char*                name,
	     size_t               minContainerSize,
	     size_t               initBlockSize,
	     size_t               maxBlockSize);

	void* (*allocateMemory)(
         void*                self,
         MemoryContainer      container, 
	     size_t               size);

	void (*resetMemoryFromSet)(
	     void*                self,
	     MemorySet            set);

	void (*freeChunk)(
         void*                self,
	     void*                mem);

} SIMemContainerManager, *IMemContainerManager;

#endif
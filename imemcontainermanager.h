#include "common.h"
#include "memory.h"
#include "ierrorlogger.h"

#ifndef IMEM_CONTAINER_MANAGER_H
#define IMEM_CONTAINER_MANAGER_H

typedef struct SIMemContainerManager
{
	IErrorLogger  errorLogger;

	void (*ctorMemContMan)(void* self);

	MemoryContainer (*memContCreate)(
	     void*                self,
         MemoryContainer      container,
	     MemoryContainer      parent,
         MemContType          type, 
	     size_t               size,
	     char*                name,
		 void*                (*malloc)(size_t size));

	MemorySet (*memSetCreate)(
         void*                self,
	     MemoryContainer      container,
         MemoryContainer      parent,
         char*                name,
	     size_t               minContainerSize,
	     size_t               initBlockSize,
	     size_t               maxBlockSize,
	     void*                (*malloc)(size_t size));

	void* (*allocateMemory)(
         void*                self,
         MemoryContainer      container, 
	     size_t               size);

	void (*resetMemoryFromSet)(MemorySet set);

} SIMemContainerManager, *IMemContainerManager;

#endif
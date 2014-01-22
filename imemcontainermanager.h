#include "common.h"
#include "memory.h"
#include "ierrorlogger.h"

#ifndef IMEM_CONTAINER_MANAGER_H
#define IMEM_CONTAINER_MANAGER_H

typedef struct SIMemContainerManager
{
	IErrorLogger  errorLogger;

	MemoryContainer (*memContCreate)(
	     void*                self,
         MemoryContainer      container,
	     MemoryContainer      parent,
         MemContType          type, 
	     size_t               size,
	     char*                name);

} SIMemContainerManager, *IMemContainerManager;

#endif
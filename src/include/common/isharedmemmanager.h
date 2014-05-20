
#include "sharedmem.h"
#include "ierrorlogger.h"
#include "ispinlockmanager.h"
#include "imemorymanager.h"

#ifndef ISHAREDMEM_H
#define ISHAREDMEM_H

typedef struct SISharedMemManager
{
	IErrorLogger      errorLogger;
	IMemoryManager    memManager;
    ISpinLockManager  spinLockMan; 
    
	void (*sharMemCtor)(
	           void*           self);

	SharMemHeader (*sharMemCreate)(
	           void*         self,
	           size_t        size);

	void (*initSharMemAccess)(void* sharMem);

	SharMemHeader (*openSharedMemSegment)(
               void*         self,
	           char*         name,
	           Bool          reportError,
			   size_t        size);
    
	Bool (*reserveSharedMemoryRegion)(
               void*           self,
	           TSharMemHandler segmId);

	SharMemHeader (*sharedMemoryReAttach)(
               void*           self,
	           void*           mem,
	           TSharMemHandler segId);

	void (*detachSharedMemory)(
	           void*           self,
	           void*           sharMem);

	void (*deleteSharedMemory)(
	           void*           self,
	           void*           sharMem,
	           TSharMemHandler sharMemHandle);

	void* (*allocSharedMem)(
	           void*       self,
	           size_t      size);

	size_t (*sizeMultiply)(
	           void*         self,
	           size_t        s1, 
	           size_t        s2);

	size_t (*addSize)(
               void*         self,
               size_t        s1, 
	           size_t        s2);

} SISharedMemManager, *ISharedMemManager;

#endif
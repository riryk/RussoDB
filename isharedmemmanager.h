
#include "sharedmem.h"
#include "ierrorlogger.h"

#ifndef ISHAREDMEM_H
#define ISHAREDMEM_H

typedef struct SISharedMemManager
{
	IErrorLogger      errorLogger;
    ISpinLockManager  spinLockMan; 

	SharMemHeader (*sharMemCreate)(
	           void*         self,
	           size_t        size, 
	           Bool          makePrivate, 
	           int           port);

	void (*initSharMemAccess)(void* sharMem);

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
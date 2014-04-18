
#include "sharedmem.h"
#include "ierrorlogger.h"

#ifndef ISHAREDMEM_H
#define ISHAREDMEM_H

typedef struct SISharedMemManager
{
	IErrorLogger  errorLogger;

	SharMemHeader (*sharMemCreate)(
	           void*         self,
	           size_t        size, 
	           Bool          makePrivate, 
	           int           port);

	void (*initSharMemAccess)(void* sharMem);

} SISharedMemManager, *ISharedMemManager;

#endif
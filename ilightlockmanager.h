
#include "common.h"
#include "ispinlockmanager.h"

#ifndef ILIGHTLOCKMANAGER_H
#define ILIGHTLOCKMANAGER_H


typedef struct SILightLockManager
{
	IErrorLogger       errorLogger;
	ISharedMemManager  sharedMemManager; 
	ISpinLockManager   spinLockManager;

	void (*lightLockAcquire)(
		void*               self,
		ELightLockType      type, 
		ELightLockMode      mode);

} SILightLockManager, *ILightLockManager;


#endif
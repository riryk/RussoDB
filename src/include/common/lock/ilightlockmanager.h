
#include "common.h"
#include "ispinlockmanager.h"
#include "lightlock.h"
#include "isemaphorelockmanager.h"

#ifndef ILIGHTLOCKMANAGER_H
#define ILIGHTLOCKMANAGER_H


typedef struct SILightLockManager
{
	IErrorLogger           errorLogger;
	ISharedMemManager      sharedMemManager; 
	ISpinLockManager       spinLockManager;
	ISemaphoreLockManager  semLockManager;

	void (*lightLockAcquire)(
		void*               self,
		ELightLockType      type, 
		ELightLockMode      mode);

	void (*lightLockRelease)(
        void*               self,
	    ELightLockType      type);

} SILightLockManager, *ILightLockManager;


#endif
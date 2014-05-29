
#include "common.h"
#include "spin.h"

#ifndef ISPINLOCKSTRATEGY_H
#define ISPINLOCKSTRATEGY_H

typedef struct SISpinLockStrategy
{
	void (*spinLockStratCtor)(void* self);
	void (*initLock)         (volatile TSpinLock* lock);
    int  (*tryLock)          (volatile TSpinLock* lock);
    void (*delay)            ();    
	Bool (*freeLock)         (volatile TSpinLock* lock);
	void (*unlock)           (volatile TSpinLock* lock);
} SISpinLockStrategy, *ISpinLockStrategy;

#endif
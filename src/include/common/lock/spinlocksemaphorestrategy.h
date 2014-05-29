
#include "ispinlockstrategy.h"
#include "isemaphorelockmanager.h"

#ifndef SPINLOCKSEMAPHORESTRATEGY_H
#define SPINLOCKSEMAPHORESTRATEGY_H

extern const SISpinLockStrategy     sSpinLockSemaphoreStrategy;
extern const ISpinLockStrategy      spinLockSemaphoreStrategy;
extern const ISemaphoreLockManager  semLockMan;

void spinLockSemStratCtor(void* self);
void initLock_semaphore(volatile TSpinLock* lock);
int tryLock_semaphore(volatile TSpinLock* lock);
void delay_semaphore();
Bool freeLock_semaphore(volatile TSpinLock* lock);
void unlock_semaphore(volatile TSpinLock* lock);

#endif

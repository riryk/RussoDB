#include "semaphorelockmanager.h"
#include "spin.h"
#include "spinlocksemaphorestrategy.h"

const SISpinLockStrategy sSpinLockSemaphoreStrategy = 
{ 
    spinLockSemStratCtor,
	initLock_semaphore,
    tryLock_semaphore,
    delay_semaphore,
	freeLock_semaphore,
    unlock_semaphore
};

const ISpinLockStrategy     spinLockSemaphoreStrategy = &sSpinLockSemaphoreStrategy;
const ISemaphoreLockManager semLockMan                = &sSemaphoreLockManager;

void spinLockSemStratCtor(void* self)
{
	semLockMan->semaphoresCtor(semLockMan, 100, 0);
}

void initLock_semaphore(volatile TSpinLock* lock) 
{
    semLockMan->semaphoreCreate(
        semLockMan,
	    (TSemaphore)lock);
}

int tryLock_semaphore(volatile TSpinLock* lock) 
{
    return semLockMan->tryLockSemaphore(
               semLockMan,
               (TSemaphore)lock);
}

void delay_semaphore() { }

Bool freeLock_semaphore(volatile TSpinLock* lock) 
{
    return True;
}

void unlock_semaphore(volatile TSpinLock* lock) 
{
	semLockMan->unlockSemaphore(
        semLockMan,
        (TSemaphore)lock);
}


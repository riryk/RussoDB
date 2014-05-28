#include "semaphorelockmanager.h"
#include "spin.h"
#include "spinlockstrategy.h"

void initLock_semaphore(volatile TSpinLock* lock);
int tryLock_semaphore(volatile TSpinLock* lock);
void delay_semaphore();
Bool freeLock_semaphore(volatile TSpinLock* lock);
void unlock_semaphore(volatile TSpinLock* lock); 

const SISpinLockStrategy sSpinLockSemaphoreStrategy = 
{ 
	initLock_semaphore,
    tryLock_semaphore,
    delay_semaphore,
	freeLock_semaphore,
    unlock_semaphore
};

const ISpinLockStrategy     spinLockSemaphoreStrategy = &sSpinLockSemaphoreStrategy;
const ISemaphoreLockManager semLockMan                = &sSemaphoreLockManager;

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
    semLockMan->semaphoreCreate(
        semLockMan,
	    (TSemaphore)lock);
}


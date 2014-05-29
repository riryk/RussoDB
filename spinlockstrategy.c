#include "spinlockstrategy.h"

const SISpinLockStrategy sSpinLockStrategy = 
{ 
	spinLockCtor_spinlock,
	initLock_spinlock,
    tryLock_spinlock,
    delay_spinlock,
	freeLock_spinlock,
    unlock_spinlock
};

const ISpinLockStrategy spinLockStrategy = &sSpinLockStrategy;

void spinLockCtor_spinlock(void* self) { }

void initLock_spinlock(volatile TSpinLock* lock) 
{
	*lock = 0;
}

#ifdef _WIN32

int tryLock_spinlock(volatile TSpinLock* lock) 
{
	return InterlockedCompareExchange(lock, 1, 0);
}

#endif

void delay_spinlock() 
{
	__asm rep nop;
}

Bool freeLock_spinlock(volatile TSpinLock* lock) { }

void unlock_spinlock(volatile TSpinLock* lock) 
{
    *lock = 0;   
}


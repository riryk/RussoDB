
const SISpinLockStrategy sSpinLockSemaphoreStrategy = 
{ 
	initLock_semaphore,
    tryLock_semaphore,
    delay_semaphore,
	freeLock_semaphore,
    unlock_semaphore
};

const ISpinLockStrategy spinLockSemaphoreStrategy = &sSpinLockSemaphoreStrategy;

void initLock_semaphore(volatile TSpinLock* lock) 
{


	*lock = 0;
}

int tryLock_semaphore(volatile TSpinLock* lock) 
{
	return InterlockedCompareExchange(lock, 1, 0);
}

void delay_semaphore() 
{
	__asm rep nop;
}

Bool freeLock_semaphore(volatile TSpinLock* lock) 
{

}

void unlock_semaphore(volatile TSpinLock* lock) 
{
    *lock = 0;   
}


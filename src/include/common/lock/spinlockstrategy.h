
#include "ispinlockstrategy.h"

extern const SISpinLockStrategy sSpinLockStrategy;
extern const ISpinLockStrategy  spinLockStrategy;

void initLock_spinlock(volatile TSpinLock* lock);
int tryLock_spinlock(volatile TSpinLock* lock);
void delay_spinlock();
Bool freeLock_spinlock(volatile TSpinLock* lock);
void unlock_spinlock(volatile TSpinLock* lock);


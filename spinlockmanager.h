
#include "ispinlockmanager.h"

#ifndef SPINLOCKMANAGER_H
#define SPINLOCKMANAGER_H

typedef unsigned int SpinLockType;

#define SPIN_LOCK_ACQUIRE(manager, lock) \
    (manager)->spinLockAcquire( \
	           (manager), \
	           (lock), \ 
	           __FILE__, \ 
	           __LINE__); \    

#define SPIN_LOCK_RELEASE(manager, lock) \
	(manager)->spinLockRelease(lock); \

int spinLockAcquire(
	  void*             self,
	  volatile long*    lock, 
	  char*             file, 
	  int               line);

void spinLockRelease(volatile long* lock);

#endif
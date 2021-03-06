
#include "ispinlockmanager.h"

#ifndef SPINLOCKMANAGER_H
#define SPINLOCKMANAGER_H

#define SPINS_DEFAULT_NUM   100
#define SLEEPS_MAX_COUNT	1000
#define SLEEP_MIN           (1 * 1000) 
#define SLEEP_MAX           (1000 * 1000)
#define MAX_RANDOM_VALUE    (0x7FFFFFFF)

#define SPINS_MIN_NUM       10
#define SPINS_MAX_NUM       1000

#define SPIN_LOCK_ACQUIRE(man, lock) \
    (man)->spinLockAcquire((man), (lock), __FILE__, __LINE__);   

#define SPIN_LOCK_RELEASE(manager, lock) \
	(manager)->spinLockRelease((manager), (lock)); \

extern const SISpinLockManager sSpinLockManager;
extern const ISpinLockManager  spinLockManager;

extern int        spinsAllowedCount;
extern sleepFunc  slpSpinFunc; 
extern int        spinsMinNum;
extern int        spinsMaxNum;

void spinLockCtor(
      void*             self,
      sleepFunc         slpFuncParam);

void spinLockInit(
	  void*             self,
	  volatile long*    lock);

int spinLockAcquire(
	  void*             self,
	  volatile long*    lock, 
	  char*             file, 
	  int               line);

void spinLockRelease(
      void*             self,
      volatile long*    lock);

#endif
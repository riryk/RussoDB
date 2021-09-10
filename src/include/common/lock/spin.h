#include "common.h"

#ifndef SPIN_H
#define SPIN_H

typedef long TSpinLock;

#define SpinLockInit(lock, nested) s_init_lock_sema(lock, nested);

#define SpinLockAcquire(lock) s_lock(lock);

#define SpinLockRelease(lock) s_release(lock);

void s_init_lock_sema(volatile int *lock, int nested);

void s_lock(volatile int *lock);

void s_release(volatile int *lock);

#endif
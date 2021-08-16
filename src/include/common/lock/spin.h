#include "common.h"

#ifndef SPIN_H
#define SPIN_H

typedef long TSpinLock;

#define SpinLockInit(lock, nested) s_init_lock_sema(lock, nested);

void s_init_lock_sema(volatile int *lock, int nested);

#endif
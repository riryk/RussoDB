
#include "common.h"

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#ifdef _WIN32

typedef HANDLE* TSemaphore;

#define EIDRM 4096
#define SEMAPHORE_WAIT_TIMEOUT 1000000

#endif

#endif
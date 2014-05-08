
#include "common.h"

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#ifdef _WIN32

typedef HANDLE* TSemaphore;

#define EIDRM 4096

#endif

#endif

#include "common.h"

#ifndef SEMAPHORELOCKMANAGER_H
#define SEMAPHORELOCKMANAGER_H

#ifdef _WIN32

typedef HANDLE* TSemaphore;

#endif

void lockSemaphore(
    void*          self,
	TSemaphore     sem);

#endif
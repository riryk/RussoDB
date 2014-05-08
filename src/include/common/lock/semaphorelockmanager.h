
#include "semaphore.h"
#include "isemaphorelockmanager.h"

#ifndef SEMAPHORELOCKMANAGER_H
#define SEMAPHORELOCKMANAGER_H

void lockSemaphore(
    void*          self,
	TSemaphore     sem);

void unlockSemaphore(
    void*          self,
	TSemaphore     sem);

#endif
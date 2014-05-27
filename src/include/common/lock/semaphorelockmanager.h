
#include "semaphore.h"
#include "isemaphorelockmanager.h"

#ifndef SEMAPHORELOCKMANAGER_H
#define SEMAPHORELOCKMANAGER_H

extern const SISemaphoreLockManager sSemaphoreLockManager;
extern const ISemaphoreLockManager  semaphoreLockManager;

void lockSemaphore(
    void*          self,
	TSemaphore     sem);

void unlockSemaphore(
    void*          self,
	TSemaphore     sem);

void semaphoresCtor(
	void*          self,
	int            semasMax, 
	int            port);

void semaphoreCreate(
    void*          self,
	TSemaphore     sem);

void releaseSemaphores();

void lockSemaphore(
    void*          self,
	TSemaphore     sem);

void unlockSemaphore(
    void*          self,
	TSemaphore     sem);

#endif
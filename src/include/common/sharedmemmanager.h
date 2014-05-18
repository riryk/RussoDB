#include "isharedmemmanager.h"

void sharMemCtor(
	void*           self);

SharMemHeader sharMemCreate(
	void*           self,
	size_t          size);

void detachSharedMemory(
	void*           self,
	void*           sharMem);

void initSharMemAccess(void* sharMem);

void* allocSharedMem(
	void*        self,
	size_t       size);

SharMemHeader sharedMemoryReAttach(
    void*           self,
	void*           mem,
	TSharMemHandler segId);

TSharMemHandler openSharedMemSegment(
    void*       self,
	char*       name,
	Bool        reportError);

void deleteSharedMemory(
	void*           self,
	void*           sharMem,
	TSharMemHandler sharMemHandle);

size_t sizeMultiply(
	void*        self,
	size_t       s1, 
	size_t       s2);

size_t addSize(
    void*        self,
    size_t       s1, 
	size_t       s2);
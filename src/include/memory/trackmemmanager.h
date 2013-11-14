
#include "imemorymanager.h"

#ifndef TRACK_MEMORY_MANAGER_H
#define TRACK_MEMORY_MANAGER_H

extern const SIMemoryManager sTrackMemManager;
extern const IMemoryManager trackMemManager;

void* memAllocStorage(uint size);
void memFreeStorage(void* mem);
void memFreeAll();
void* memRealloc(void* newMem, uint size);

#endif
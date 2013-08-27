
#include "common.h"

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#define MEM_STORAGE_SIZE 200

extern void* memStorage[MEM_STORAGE_SIZE];
extern uint  memStorageCount;

void* memAlloc(uint size);

#endif
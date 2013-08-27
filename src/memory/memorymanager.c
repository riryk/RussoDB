#include "memorymanager.h"

void* memStorage[MEM_STORAGE_SIZE];
uint  memStorageCount = 0;

void* memAlloc(uint size)
{
   void* mem = malloc(size);
   if (mem != NULL)
       memStorage[memStorageCount++] = mem;
   return mem;
}
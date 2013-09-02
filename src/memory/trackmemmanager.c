#include "trackmemmanager.h"

void* memAllocStorage(uint size);
void memFreeStorage(void* mem);
void memFreeAll();

const SIMemoryManager sTrackMemManager = 
{
   memAllocStorage,
   memFreeStorage,
   memFreeAll
};

const IMemoryManager trackMemManager = &sTrackMemManager;


#define MEM_STORAGE_SIZE 200

void* memStorage[MEM_STORAGE_SIZE];
uint  memStorageCount = 0;

void* memAllocStorage(uint size)
{
   void* mem = malloc(size);
   if (mem != NULL)
       memStorage[memStorageCount++] = mem;
   return mem;
}

void memFreeStorage(void* mem)
{
   free(mem);
}

void memFreeAll()
{
   int i;
   for (i = 0; i < memStorageCount; i++)
   {
	   free(memStorage[i]);
       memStorage[i] = 0;
   }
   memStorageCount = 0;
}
#include "trackmemmanager.h"

const SIMemoryManager sTrackMemManager = 
{
   memAllocStorage,
   memRealloc,
   memFreeStorage,
   memFreeAll
};

const IMemoryManager trackMemManager = &sTrackMemManager;


#define MEM_STORAGE_SIZE 200

void* memStorage[MEM_STORAGE_SIZE];
uint  memStorageAddrs[MEM_STORAGE_SIZE];
uint  memStorageCount = 0;

void* memAllocStorage(uint size)
{
   void* mem = malloc(size);
   if (mem != NULL)
   {
	   int i = memStorageCount;
       memStorage[i] = mem;
       memStorageAddrs[i] = (uint)mem;
       memStorageCount++;
   }
   return mem;
}

int findMemInd(void* mem)
{
   int i;
   for (i = 0; i < MEM_STORAGE_SIZE; i++)
       if (memStorageAddrs[i] == (uint)mem)
		   return i;
   return -1;
}

void* memRealloc(void* newMem, uint size)
{
   int   memInd = findMemInd(newMem);
   void* mem    = realloc(newMem, size);
   if (mem != NULL)
       memStorage[memInd] = mem;
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
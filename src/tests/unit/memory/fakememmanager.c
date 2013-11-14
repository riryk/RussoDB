
#include "fakememmanager.h"

void* allocFake(uint size);
void freeFake(void* mem);
void freeAllFake();
void* reallocFake(void* mem, uint size);

const SIMemoryManager sFakeMemManager = 
{
   allocFake,
   reallocFake,
   freeFake,
   freeAllFake
};

const IMemoryManager fakeMemManager = &sFakeMemManager;

SMemHistoryItem fakeMemStorageBackup[10000];
uint fakeMemStorageCountBackup = 0;

SMemHistoryItem fakeMemStorage[10000];
uint fakeMemAddresses[10000];
uint fakeMemStorageCount = 0;

void ffreeAllFake(MemHistoryItem arr, int* arrCount)
{
   int i;
   for (i = 0; i < *arrCount; i++)
   {
	   free(arr[i].mem);
	   arr[i].mem = 0x0;
	   arr[i].memLen = 0;
   }
   *arrCount = 0;
}

void backupMemory()
{
   int i;
   for (i = 0; i < fakeMemStorageCount; i++)
   {
	   fakeMemStorageBackup[i].mem = fakeMemStorage[i].mem;
       fakeMemStorage[i].mem = NULL;

       fakeMemStorageBackup[i].memLen = fakeMemStorage[i].memLen;
       fakeMemStorage[i].memLen = 0;
   }
   fakeMemStorageCountBackup = fakeMemStorageCount;
   fakeMemStorageCount = 0;
}

void* allocFake(uint size)
{
   void* mem = malloc(size);
   if (mem != NULL)
   {
	   int i = fakeMemStorageCount;
	   MemHistoryItem item = &fakeMemStorage[i];
       item->mem = mem;
       item->memLen = size;
	   fakeMemAddresses[i] = (uint)mem;
       fakeMemStorageCount++;
   }
   return mem;
}

int findMemIndFake(void* mem)
{
   int i;
   for (i = 0; i < fakeMemStorageCount; i++)
       if (fakeMemAddresses[i] == (uint)mem)
		   return i;
   return -1;
}

void* reallocFake(void* mem, uint size)
{
   int   ind    = findMemIndFake(mem);
   void* newmem = realloc(mem, size);
   if (newmem != NULL)
   {
	   fakeMemStorage[ind].mem = newmem;
	   fakeMemStorage[ind].memLen = size;
   }
   return newmem;
}

void freeFake(void* mem)
{
   free(mem);
}

void freeAllFake()
{
   ffreeAllFake(fakeMemStorage, &fakeMemStorageCount);
   ffreeAllFake(fakeMemStorageBackup, &fakeMemStorageCountBackup);
}
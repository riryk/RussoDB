
#include "fakememmanager.h"

void* allocFake(uint size);
void freeFake(void* mem);
void freeAllFake();

const SIMemoryManager sFakeMemManager = 
{
   allocFake,
   freeFake,
   freeAllFake
};

const IMemoryManager fakeMemManager = &sFakeMemManager;

SMemHistoryItem fakeMemStorageBackup[200];
uint fakeMemStorageCountBackup = 0;

SMemHistoryItem fakeMemStorage[200];
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
	   MemHistoryItem item = &fakeMemStorage[fakeMemStorageCount];
       item->mem = mem;
       item->memLen = size;
       fakeMemStorageCount++;
   }
   return mem;
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

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

void BackupMemory()
{
   
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
   int i;
   for (i = 0; i < fakeMemStorageCount; i++)
   {
	   free(fakeMemStorage[i].mem);
	   fakeMemStorage[i].mem = 0x0;
	   fakeMemStorage[i].memLen = 0;
   }
   fakeMemStorageCount = 0;
}
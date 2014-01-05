
#ifndef IBUFFER_MANAGER_H
#define IBUFFER_MANAGER_H

#include "hashtable.h"
#include "buffer.h"
#include "ilatchmanager.h"
#include "latch.h"

typedef struct SIBufferManager
{   
	IHashtableManager  hashtableManager;
	IRelFileManager    relFileManager;
	ILatchManager      latchManager;
	IMemoryManager     memoryManager;
	void*              relationManager;
    ICommon            commonManager;

	void (*ctorBufMan)(void* self);

	Bool (*pinBuffer)(
         void*                self,
         BufferInfo           buf, 
	     BufRing              ring);

	void (*unpinBuffer)(
	     void*                self,
	     BufferInfo           buf);

	BufferInfo (*getBufferFromRing)(
	     void*                self,
	     BufRing              ring);

	BufferInfo (*getBufferFromRingArray)(BufRing ring);

	void (*flushBuffer)(
		 void*                self,
		 BufferInfo           buf, 
		 RelData              rel);

	int (*readBuffer)(
	     void*                self,
         RelData              rel, 
         FilePartNumber       partnum,
         uint                 blocknum,
         BufRing              ring);
    
	int (*getBlockNum)(int buffer);
    
	void (*dirtyBuffer)(int buffer);
    
	uint (*getBlocksNumInRelPart)(
	     void*                self,
	     RelData              rel, 
	     FilePartNumber       pnum);

} SIBufferManager, *IBufferManager;

#endif





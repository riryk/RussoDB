
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

} SIBufferManager, *IBufferManager;

#endif





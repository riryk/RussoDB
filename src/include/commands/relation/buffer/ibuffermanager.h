
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
} SIBufferManager, *IBufferManager;

#endif





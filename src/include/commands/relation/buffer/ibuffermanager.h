
#ifndef IBUFFER_MANAGER_H
#define IBUFFER_MANAGER_H

#include "hashtable.h"
#include "buffer.h"

typedef struct SIBufferManager
{   
	IHashtableManager hashtableManager;
	IRelFileManager relFileManager;
} SIBufferManager, *IBufferManager;

#endif





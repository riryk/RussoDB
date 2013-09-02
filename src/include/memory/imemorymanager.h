#include "common.h"

#ifndef IMEMORY_MANAGER_H
#define IMEMORY_MANAGER_H


/* This struct represents an interface for our memory 
 * namager. We can have a lot of different implementations for 
 * the IMemoryManager: 
 *   1. For keeping a track of all memory allocs.
 *   2. A mocked version for unit tests. 
 *   3. A version with a freeList, where we do not give memory back
 *      to an operation system, but keep it and recycle later.
 * This is a general interface for all memory operations 
 */
typedef struct SIMemoryManager
{
    void* (*alloc)(uint size);	
	void (*free)(void* mem);
	void (*freeAll)();
} SIMemoryManager, *IMemoryManager;


#endif
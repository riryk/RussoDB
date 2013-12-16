
#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "hashtable.h"
#include "relationmanager.h"
#include "ibuffermanager.h"

extern Hashtable    bufCache;

extern const SIBufferManager sBufferManager;
extern const IBufferManager  bufferManager;

Bool pinBuffer(
    void*                self,
	BufferInfo           buf, 
	BufferAccessStrategy strategy);

void unpinBuffer(
	void*                self,
	BufferInfo           buf);

BufferInfo allocateBuffer(
    void*             self,
    RelData           rel,
    char              relPersist, 
    FilePartNumber    partNum,
    uint              blockNum);

#endif
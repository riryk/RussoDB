
#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "hashtable.h"
#include "relationmanager.h"
#include "ibuffermanager.h"

/* This buffer manager is applied for writing 
 * buffers to a database file. It is organized 
 * as buffer pool, buffer cache. We can read it 
 * from this link:
 * http://www.westnet.com/~gsmith/content/postgresql/InsideBufferCache.pdf
 */
extern Hashtable    bufCache;

extern const SIBufferManager sBufferManager;
extern const IBufferManager  bufferManager;

extern BufFreeListState  freeBufferState;
extern BufferInfo        bufInfos;

void ctorBufMan(void* self);

Bool pinBuffer(
    void*                self,
    BufferInfo           buf, 
	BufRing              ring);

void unpinBuffer(
	void*                self,
	BufferInfo           buf);

BufferInfo allocateBuffer(
    void*                self,
    RelData              rel,
    char                 relPersist, 
    FilePartNumber       partNum,
    uint                 blockNum,
	BufRing              ring);

BufferInfo getBufferFromRing(
	void*                self,
	BufRing              ring);

BufferInfo getBufferFromRingArray(BufRing ring);

void flushBuffer(
    void*                self,
    BufferInfo           buf, 
	RelData              rel);

int readBuffer(
	void*                self,
    RelData              rel, 
    FilePartNumber       partnum,
    uint                 blocknum,
    BufRing              ring);

int getBlockNum(int buffer);

#endif
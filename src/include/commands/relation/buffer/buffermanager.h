
#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "hashtable.h"
#include "relationmanager.h"
#include "ibuffermanager.h"
#include "page.h"
#include "relfile.h"

/* This buffer manager is applied for writing 
 * buffers to a database file. It is organized 
 * as buffer pool, buffer cache. We can read it 
 * from this link:
 * http://www.westnet.com/~gsmith/content/postgresql/InsideBufferCache.pdf
 */
typedef void *Block;

extern Hashtable    bufCache;

extern const SIBufferManager sBufferManager;
extern const IBufferManager  bufferManager;

extern BufFreeListState  freeBufferState;
extern BufferInfo        bufInfos;
extern char*             bufBlocks;
extern int			     bufNum;

void ctorBufMan(void* self);

BufferId ReadBuffer(Relation reln, BlockNumber blockNum);

void ReleaseBuffer(int buffer);

#define BufferGetBlock(buffer) ((Block) (bufBlocks + ((uint) ((buffer) - 1)) * BlockSize))

#define BufferGetPage(buffer) ((Page)BufferGetBlock(buffer))

#define RelationGetNumberOfBlocks(relation) \
   RelationGetNumberOfBlocksInFork(relation, FILE_PART_MAIN)

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
	BufRing              ring,
	Bool*                found);

BufferInfo getBufferFromRing(
	void*                self,
	BufRing              ring);

BufferInfo getBufferFromRingArray(BufRing ring);

void flushBuffer(
    void*                self,
    BufferInfo           buf, 
	RelData              rel);

int readBufferInternal(
	void*                self,
    RelData              rel, 
    FilePartNumber       partnum,
    uint                 blocknum,
    BufRing              ring);

int BufferGetBlockNumber(int buffer);

void dirtyBuffer(int buffer);

BlockNumber RelationGetNumberOfBlocksInFork(Relation relation, FilePartNumber forkNumber);

uint getBlocksNumInRelPart(
	void*                 self,
	RelData               rel, 
	FilePartNumber        pnum);

#endif
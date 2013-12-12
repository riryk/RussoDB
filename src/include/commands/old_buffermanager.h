
#ifndef BUFFER_MAN_H
#define BUFFER_MAN_H

#include <stdio.h>
#include "tranlog.h"


struct BufferDescription
{
	int            Id;
	unsigned int   blockNumber;
	unsigned short flags;
	struct RelationFileInfo    relation;	
	Fork	       forkNum;
	unsigned int         blockNum;
};

/*
 * The shared freelist control information.
 */
struct BufferState
{
	int			nextBufferToFlush;
	int			firstFreeBuffer;	
	int			lastFreeBuffer;
	unsigned int		completedCycles; 
	unsigned int		newBufferAllocs;
};

struct BufferWriterState
{
    int         numberBuffersAlloc;
};

extern  struct BufferDescription* LocalBufferDescriptions;
extern  struct BufferDescription* BufferDescriptions;

extern void** LocalBuffers;
extern char* BufferBlocks;


#endif
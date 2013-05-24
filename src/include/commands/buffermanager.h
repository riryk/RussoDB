
#include <stdio.h>

struct BufferDescription
{
	int            Id;
	unsigned int   blockNumber;
	unsigned short flags;
};

/*
 * The shared freelist control information.
 */
struct BufferState
{
	int			nextBufferToFlush;
	int			firstFreeBuffer;	
	int			lastFreeBuffer;
	uint32		completedCycles; 
	uint32		newBufferAllocs;
};

struct BufferWriterState
{
    int         numberBuffersAlloc;
};
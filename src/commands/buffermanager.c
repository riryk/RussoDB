#include "buffermanager.h"

struct BufferDescription* LocalBufferDescriptions = NULL;
void** LocalBuffers = NULL;

struct BufferDescription* BufferDescriptions = NULL;
char* BufferBlocks = NULL;

void* GetBlock(int buffer)
{
    if (buffer < 0)
		return (void*)LocalBuffers[-buffer - 1];

    return (void*)BufferBlocks[buffer - 1];
}

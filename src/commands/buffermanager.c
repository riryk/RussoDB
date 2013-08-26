#include "buffermanager.h"
#include "relation_storage.h"

int			NBuffers = 1000;

struct BufferDescription* LocalBufferDescriptions = NULL;
void** LocalBuffers = NULL;

struct BufferDescription* BufferDescriptions = NULL;
char* BufferBlocks = NULL;




/* Pointers to shared state */
static struct BufferState* BufferStatePointer = NULL;
static struct BufferWriterState* BufferWriterStatePointer = NULL;

void* GetBlock(int buffer)
{
    if (buffer < 0)
		return (void*)LocalBuffers[-buffer - 1];

    return (void*)BufferBlocks[buffer - 1];
}

int BgBufferSync(void)
{ 
	int			bufferIdToProcess;
	unsigned int		completedCycles; 
	unsigned int		newBufferAllocs;

    unsigned int  	prevbufferId;
	unsigned int      prevCompletedCycles;

	int			pendingBuffers;
	int			reusableBuffers;

	float       smoothedAlloc = 0;
	int			upcomingAlloc;
	int         minBufferToScan;

    int         loopNumToScan;
	int         loopNumWritten;
	int         loopReusableBuffer;

	bufferIdToProcess = BufferStatePointer->nextBufferToFlush;
	completedCycles = 12; //StrategyControl->completedCycles;
	newBufferAllocs = 23; //StrategyControl->newBufferAllocs;

	BufferWriterStatePointer->numberBuffersAlloc += newBufferAllocs;

	prevbufferId = bufferIdToProcess;
	prevCompletedCycles = completedCycles;

	pendingBuffers = 0;
	reusableBuffers = 0;

	if (smoothedAlloc <= newBufferAllocs)
		smoothedAlloc = newBufferAllocs;

    upcomingAlloc = (int)(smoothedAlloc * 2);
	minBufferToScan = (int)(NBuffers / (120000.0 / 200));

    if (upcomingAlloc < minBufferToScan + reusableBuffers)
		upcomingAlloc = minBufferToScan + reusableBuffers;

	loopNumToScan = 0;
	loopNumWritten = 0;
	loopReusableBuffer = 0;

	while (loopNumToScan > 0 && loopReusableBuffer < upcomingAlloc)
	{
        struct StorageRelation relation;
		void* block;
        struct PageHeader* page;

        volatile struct BufferDescription* bufferDesc = &BufferDescriptions[loopNumToScan];   

		//relation = RelationOpen(bufferDesc->relation, -1);

		//recptr = BufferGetLSN(buf);

		block = (void*)(BufferBlocks + bufferDesc->Id * BLOCK_SIZE);
        page = (struct PageHeader*)block;
	}
}
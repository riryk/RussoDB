
#include "spacepartitioning.h"
#include "types.h"

uint SpGistGetTypeSize(SpacePartitioningType* type, DataPointer labelData)
{
	uint size;

	if (type->IsPassedByValue)
		size = sizeof(DataPointer);
	else if (type->TypeLength > 0)
		size = type->TypeLength;
	else
		size = ComputeSize(labelData);

	return 0;  //MAXALIGN(size);
}

SpacePartitioningNodeTuple CreateNodeTuple(SpacePartitioningState* state, DataPointer labelData, Bool isnull)
{
    
}

#include "spacepartitioning.h"
#include "types.h"

uint CalculateIndexTupleArraySize(SpacePartitioningNodeTuple* nodeArray, int numberOfNodes)
{
	int i, nodeArraySize = 0; 

	for (i = 0; i < numberOfNodes; i++)
		nodeArraySize += IndexTupleSize(nodeArray[i]);

	return nodeArraySize;
}

uint ComputeSpacePartitioningTypeSize(SpacePartitioningType* type, DataPointerIntValue labelData)
{
	if (type->IsPassedByValue)
        return AlignDefault(sizeof(DataPointer));

    if (type->TypeLength > 0)
	    return AlignDefault(type->TypeLength);

    return AlignDefault(ComputeSize(labelData));
}

static void CopyDataToMemory(void* targetMemory, SpacePartitioningType* type, DataPointerIntValue pointerToData, uint dataSize)
{
	uint dataSize;

	if (type->IsPassedByValue)
	{
		memcpy(targetMemory, &pointerToData, dataSize);
		return;
	}

    memcpy(targetMemory, ConvertIntValueToPointer(pointerToData), dataSize);
}

SpacePartitioningNodeTuple CreateNullNodeTuple()
{
	SpacePartitioningNodeTuple tuple = (SpacePartitioningNodeTuple)AllocateMemory(NodeTupleHeaderSize);
    uint16 tupleDataProperties = 0;

    tupleDataProperties |= IndexIsNullMask;
	tupleDataProperties |= NodeTupleHeaderSize;

	tuple->IndexProperties = tupleDataProperties;
	SetDefaultValueToPointerToTuple(&tuple->PointerToTuple);

    return tuple;
}

SpacePartitioningNodeTuple CreateNodeTuple(SpacePartitioningState* state, DataPointerIntValue labelData)
{
	SpacePartitioningNodeTuple tuple;

	uint nodeTupleSize = NodeTupleHeaderSize;
    uint nodeTupleDataSize = 0;
	uint16 tupleDataProperties = 0;

	nodeTupleDataSize = ComputeSpacePartitioningTypeSize(&state->LabelType, labelData);
	nodeTupleSize += nodeTupleDataSize;

	if (IndexTupleSizeExceedsLimit(nodeTupleSize))
		ReportError(ErrorLevel, "Exceeded max index size");

	tuple = (SpacePartitioningNodeTuple)AllocateMemory(nodeTupleSize);

	tupleDataProperties |= nodeTupleSize;

	tuple->IndexProperties = tupleDataProperties;
	SetDefaultValueToPointerToTuple(&tuple->PointerToTuple);

    CopyDataToMemory(InnerTupleData(tuple), &state->LabelType, labelData, nodeTupleDataSize);

	return tuple;
}

SpacePartitioningInnerTuple CreateInnerTuple(SpacePartitioningState* state, unit prefixSize, DataPointerIntValue prefixData,
				                             uint numberOfNodes, SpacePartitioningNodeTuple* nodes)
{
    SpacePartitioningInnerTuple innerTuple;
	SpacePartitioningNodeTuple nodeTuple;

	uint i = 0;
	uint innerTupleSize = InnerTupleHeaderSize + prefixSize + CalculateIndexTupleArraySize(nodes, numberOfNodes);

	if (InnerTupleSizeExceedsPageLimit(innerTupleSize))
	    ReportError(ErrorLevel, "Exceeded max page size limit");	 

	innerTuple = (SpacePartitioningInnerTuple)AllocateMemory(innerTupleSize);

	innerTuple->NumberOfNodes = numberOfNodes;
	innerTuple->PrefixSize = prefixSize;
	innerTuple->Size = innerTupleSize;

    CopyDataToMemory(InnerTupleData(innerTuple), &state->PrefixType, prefixData, prefixSize);

	IterateThruInnerTupleNodes(innerTuple, i, nodeTuple)
	{
	   memcpy(nodeTuple, nodes[i], IndexTupleSize(nodes[i]));   	 
	}

	return innerTuple;
}
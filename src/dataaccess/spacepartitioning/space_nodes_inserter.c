#include "spacepartitioning.h"

static SpacePartitioningNodeTuple FindNode(SpacePartitioningInnerTuple innerTuple, int nodeNumber)  
{
	int i;
    SpacePartitioningNodeTuple outputNode;

	IterateThruInnerTupleNodes(innerTuple, i, outputNode)
	{
		if (i == nodeNumber)
		    return outputNode;
	}
}

static int CompareOffsetNumbers(void* leftNumber, void* rightNumber)
{
	if (*(PageItemOffset*)leftNumber == *(PageItemOffset*)rightNumber)
		return QuickSortIsEqual;

	return (*(PageItemOffset*)leftNumber > *(PageItemOffset*)rightNumber) ? QuickSortIsLarger : QuickSortIsSmaller;
}

SpacePartitioningNodeTuple* InsertNodeAndFormNewArray(SpacePartitioningInnerTuple innerTuple, 
						 			 		          SpacePartitioningNodeTuple newNode, int newNodePosition)
{
	int i;
    SpacePartitioningNodeTuple nodeTuple;
	SpacePartitioningNodeTuple* newNodesArray = AllocateMemory(sizeof(SpacePartitioningNodeTuple) * innerTuple->NumberOfNodes + 1);

	IterateThruInnerTupleNodes(innerTuple, i, nodeTuple)
	{
		if (i < newNodePosition)
			newNodesArray[i] = nodeTuple;
		else
			newNodesArray[i + 1] = nodeTuple; 
	}

	return newNodesArray;
}

void UpdateNodePointer(SpacePartitioningInnerTuple innerTuple, int nodeNumber,
				       BlockNumber blockNumber, PageItemOffset offset)
{
	SpacePartitioningNodeTuple nodeTuple = FindNode(innerTuple, nodeNumber);

    SetItemPointerTo(&nodeTuple->PointerToTuple, blockNumber, offset);
}

SpacePartitioningInnerTuple AddNode(SpacePartitioningInnerTuple innerTuple, int newNodePosition,
								    DataPointerIntValue newNodeLabelData, SpacePartitioningState* state)
{
	SpacePartitioningNodeTuple newNodeTuple, *newNodeArray;
	DataPointer pointerToPrefix = InnerTupleData(innerTuple);

	if (DoesNotExceedTupleBoundaries(newNodePosition, innerTuple))
		ReportError(ErrorLevel, "Invalid newNodePositionInNodesArray");

	newNodeTuple = CreateNodeTuple(state, newNodeLabelData);
    newNodeArray = InsertNodeAndFormNewArray(innerTuple, newNodeTuple, newNodePosition);

	return CreateInnerTuple(state, innerTuple->PrefixSize, pointerToPrefix, innerTuple->NumberOfNodes, newNodeArray);
}

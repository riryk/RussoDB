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

SpacePartitioningNodeTuple InsertNodeAndFormNewArray(SpacePartitioningInnerTuple innerTuple, 
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
}

void updateNodePointer(SpacePartitioningInnerTuple innerTuple, int nodeNumber,
				       BlockNumber blockNumber, PageItemOffset pageOffset)
{
	SpacePartitioningNodeTuple nodeTuple = FindNode(innerTuple, nodeNumber);

    SetItemPointerTo(&nodeTuple->PointerToTuple, blockNumber, pageOffset);
}

SpacePartitioningInnerTuple AddNode(SpacePartitioningInnerTuple innerTuple, int newNodePosition,
								    DataPointer newNodeLabelData, SpacePartitioningState* state)
{
	if (DoesNotExceedTupleBoundaries(newNodePosition, innerTuple))
		ReportError(ErrorLevel, "Invalid newNodePositionInNodesArray");


}

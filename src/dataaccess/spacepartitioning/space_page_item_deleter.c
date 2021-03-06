#include "spacepartitioning.h"


void SpacePartitioningPageIndexDeleteMultipleItems(SpacePartitioningState* state, Page page,
						PageItemOffset* deleteItemOffsets, int numberOfDeleteItems,
						int firstDeleteItemState, int otherDeleteItemsState,
						BlockNumber blockNumber, PageItemOffset pageItemOffset)
{
	int sortedIndex;  
	PageItemOffset sortedDeleteItemOffsets[MaxRowCountPerPage];	 

    SpacePartitioningDeadTuple deadTupleForFirstPosition = CreateDeadTuple(state, firstDeleteItemState, blockNumber, pageItemOffset);
	SpacePartitioningDeadTuple deadTupleForOtherPositions = CreateDeadTuple(state, otherDeleteItemsState, blockNumber, pageItemOffset);  

	memcpy(sortedDeleteItemOffsets, deleteItemOffsets, sizeof(PageItemOffset) * numberOfDeleteItems);

	QuickSort(sortedDeleteItemOffsets, numberOfDeleteItems, sizeof(PageItemOffset), CompareOffsetNumbers);      
    PageIndexDeleteMultipleItems(page, sortedDeleteItemOffsets, numberOfDeleteItems);

	for (sortedIndex = 0; sortedIndex < numberOfDeleteItems; sortedIndex++)
	{
		PageItemOffset deleteItemOffset = sortedDeleteItemOffsets[sortedIndex]; 
		Bool isFistItem = deleteItemOffset == deleteItemOffsets[0];
		SpacePartitioningDeadTuple tupleToPut = isFistItem ? deadTupleForFirstPosition : deadTupleForOtherPositions;

		PageAddItem(page, (Item)deadTuple, deadTuple->TupleSize, deleteItemOffset, False, False);
	}   	 
}
#include "indextuple.h"
#include "error.h"
#include "memory.h"
#include "scankey.h"
#include "page.h"
#include "list.h"
#include "rel.h"

#ifndef Space_Partitioning_h
#define Space_Partitioning_h


#define SpacePartitioningLiveTupleType  	  0	
#define SpacePartitioningRedirectTupleType    1
#define SpacePartitioningDeadTupleType  	  2
#define SpacePartitioningPlaceholderType      3

typedef struct SpacePartitioningInnerTupleData
{
	unsigned int TupleState:2,	           /* Live, Redirect, Dead, Placeholder */
				 AllNodesAreTheSame:1,	   
				 NumberOfNodes:13,		   
				 PrefixSize:16;	           

	uint16		 Size;			           

} SpacePartitioningInnerTupleData;

typedef SpacePartitioningInnerTupleData *SpacePartitioningInnerTuple;

#define InnerTupleHeaderSize AlignDefault(sizeof(SpacePartitioningInnerTupleData))

#define InnerTupleData(tuple) (((char*)(tuple)) + InnerTupleHeaderSize)

#define InnerTupleStartNode(tuple) ((SpacePartitioningNodeTuple)(InnerTupleData(tuple) + (tuple)->PrefixSize))

#define IterateThruInnerTupleNodes(innerTuple, i, nodeTuple)	\
	for ((i) = 0, (nodeTuple) = InnerTupleStartNode(innerTuple); \
		 (i) < (innerTuple)->NumberOfNodes; \
		 (i)++, (nodeTuple) = (SpacePartitioningNodeTuple)(((char*)(nodeTuple)) + IndexTupleSize(nodeTuple)))

#define DoesNotExceedTupleBoundaries(position, innerTuple) \
    ((position) < 0 || (position) > (innerTuple)->NumberOfNodes)


typedef IndexTupleData SpacePartitioningNodeTupleData;

typedef SpacePartitioningNodeTupleData *SpacePartitioningNodeTuple;

#define NodeTupleHeaderSize AlignDefault(sizeof(SpacePartitioningNodeTupleData))


typedef struct SpacePartitioningDeadTupleData
{
	uint                   TupleState:2,	          /* LIVE/REDIRECT/DEAD/PLACEHOLDER */
		                   TupleSize:30;
	PageItemOffset         NextOffset;	     
	PageItemPointerData    RedirectTo;	   
	TransactionId          TransactionId;
} SpacePartitioningDeadTupleData;

typedef SpacePartitioningDeadTupleData *SpacePartitioningDeadTuple;

#define DeadTupleHeaderSize AlignDefault(sizeof(SpacePartitioningDeadTupleData))


typedef struct SpacePartitioningConfig
{
	ObjectId	InnerTuplePrefixType;		
	ObjectId	InnerTupleNodeLabelType;	
	Bool		CanReturnData;	            
	Bool		SupportLongValues;	        
} SpacePartitioningConfig;

typedef struct SpacePartitioningType
{
	ObjectId   TypeId;
	Bool       IsPassedByValue;
	int16	   TypeLength;
} SpacePartitioningType;

typedef struct SpacePartitioningState
{
	SpacePartitioningConfig InnerTupleConfig;

	SpacePartitioningType   LeafValuesType;		
	SpacePartitioningType   PrefixType;
	SpacePartitioningType   LabelType;

	char*                   DeadTupleStorage;

	TransactionId           TransactionId;		
	Bool		            PerformsIndexBuild;	
} SpacePartitioningState;


typedef struct SpacePartitioningIndexScanData
{
	SpacePartitioningState   State;
	MemoryContainer          TemporaryMemoryContainer;

	Bool		SearchNulls;
	Bool		SearchNonNulls;

	int			NumberOfIndexKeys;
	ScanKey		IndexScankeyData;

	List*       PagesToVisit;
} SpacePartitioningIndexScanData;

typedef SpacePartitioningIndexScanData* SpacePartitioningIndexScan;

#define SpacePartitioningMaxPageDataSize  \
	AlignDownDefault(BlockSize - \
				     SizeOfPageHeader - \
				     AlignDefault(sizeof(SpacePartitioningIndexScanData)))

#define InnerTupleSizeExceedsPageLimit(innerTupleSize) \
    (innerTupleSize > SpacePartitioningMaxPageDataSize - sizeof(PageItemPointerData))

typedef struct SpacePartitioningLeafTupleData
{
	uint              TupleState:2,	  /* LIVE/REDIRECT/DEAD/PLACEHOLDER */
			          Size:30;		      
	PageItemOffset    NextTupleOffset;
	ItemPointerData   PointerToHeapTuple;

} SpacePartitioningLeafTupleData;

typedef SpacePartitioningLeafTupleData* SpacePartitioningLeafTuple;

#endif





#include "indextuple.h"
#include "error.h"
#include "memory.h"

#ifndef Space_Partitioning_h
#define Space_Partitioning_h

typedef struct SpacePartitioningInnerTupleData
{
	unsigned int TupleState:2,	           /* Live, Redirect, Dead, Placeholder */
				 AllNodesAreTheSame:1,	   
				 NumberOfNodes:13,		   
				 PrefixSize:16;	           

	uint16		 Size;			           

} SpacePartitioningInnerTupleData;

typedef SpacePartitioningInnerTupleData *SpacePartitioningInnerTuple;

typedef IndexTupleData SpacePartitioningNodeTupleData;

typedef SpacePartitioningNodeTupleData *SpacePartitioningNodeTuple;

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
	SpacePartitioningType   InnerTuplePrefixValuesType;
	SpacePartitioningType   NodeLabelValuesType;

	char*                   DeadTupleStorage;

	TransactionId           CreateRedirectTupleTranId;		
	Bool		            PerformsIndexBuild;	
} SpacePartitioningState;


#define InnerTupleHeaderSize ALIGN_DEFAULT(sizeof(SpacePartitioningInnerTupleData))

#define InnerTupleData(tuple) (((char*)(tuple)) + InnerTupleHeaderSize)

#define InnerTupleStartNode(tuple) ((SpacePartitioningNodeTuple)(InnerTupleData(tuple) + (tuple)->PrefixSize))


#define IterateThruInnerTupleNodes(innerTuple, i, nodeTuple)	\
	for ((i) = 0, (nodeTuple) = InnerTupleStartNode(innerTuple); \
		 (i) < (innerTuple)->NumberOfNodes; \
		 (i)++, (nodeTuple) = (SpacePartitioningNodeTuple)(((char*)(nodeTuple)) + IndexTupleSize(nodeTuple)))

#define DoesNotExceedTupleBoundaries(position, innerTuple) \
    ((position) < 0 || (position) > (innerTuple)->NumberOfNodes)


#endif






#ifndef Space_Partitioning_h
#define Space_Partitioning_h

#include "buffer.h"
#include <io.h>

#ifdef _WIN32
#include <windows.h>
#endif

/*
 * Represents list of "nodes" that subdivide a set of tuples
 * Layout: header/optional prefix/array of nodes
 */
typedef struct SpacePartitioningInnerTupleData
{
	unsigned int TupleState:2,	           /* LIVE/REDIRECT/DEAD/PLACEHOLDER */
				 AllNodesAreTheSame:1,	   /* all nodes in tuple are equivalent */
				 NumberOfNodes:13,		   /* number of nodes within inner tuple */
				 PrefixSize:16;	           /* size of prefix, or 0 if none */

	uint16		 Size;			           /* total size of inner tuple */

	/* On most machines there will be a couple of wasted bytes here */
	/* prefix datum follows, then nodes */
} SpacePartitioningInnerTupleData;

typedef SpacePartitioningInnerTupleData *SpacePartitioningInnerTuple;

/*
 * Node tuples use the same header as ordinary IndexTuples, 
 */
typedef IndexTupleData SpacePartitioningNodeTupleData;


#define InnerTupleHeaderSize ALIGN_DEFAULT(sizeof(SpacePartitioningInnerTupleData))

#define InnerTupleData(tuple) (((char*)(tuple)) + InnerTupleHeaderSize)

#define InnetTupleStartNode(x) ((SpGistNodeTuple) (_SGITDATA(x) + (x)->prefixSize))

#endif





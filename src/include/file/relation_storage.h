
#include "hashtable.h"
#include "tranlog.h"

#define MAX_REL_PARTS_NUMBER 10
#define RELATION_SEGMENT_SIZE 131072
#define BLOCK_SIZE 8192

#define MaxHeapTupleSize BLOCK_SIZE

struct RelationFileBackend
{
	struct RelationFileInfo   fileInfo;
	int	               backend;
};

struct StorageRelation
{
   struct RelationFileBackend  relationKey;
   struct StorageRelation** Parent;
   int CurrentBlock;
   int FsmForkSize;
   int VmForkSize;
   int StorageManager;
   struct RelationSegment* Segments[4 + 1];
   struct StorageRelation* NextRelation;
};

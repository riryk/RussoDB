
#define FSM_TREE_DEPTH 4

typedef struct
{
	int			treeLevel;	
	int			number;		
} FreeSpaceNode;

static const FreeSpaceNode FSM_ROOT = {FSM_TREE_DEPTH - 1, 0};

static int GetBuffer(FreeSpaceNode freeSpaceNode)
{
   
}

int FindFreeBuffer(struct Relation relation, int spaceNeeded)
{
   FreeSpaceNode   currentNode = FSM_ROOT;

   for (;;)
   {
	     
   }
}
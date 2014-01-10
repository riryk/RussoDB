

typedef enum MemContType
{
   MCT_Invalid         = 0,
   MCT_MemoryContainer = 10
} MemContType;

/* Memory allocations are united into memory container.
 * Each container includes chunks of memory. And when we delete 
 * a container we can free all chunks which is more easily than
 * deleting chucks separately. This approach is less prone to 
 * memory leakage errors. All containers are organized into a tree
 * that keeps chunks' lifetime.
 * We should describe how memory container tree looks like:
 *  t
 *  n         -------------  next    -------------  next
 *  e     -> | parent      |------> | next parent | ----> .....
 *  r   /     -------------          -------------
 *  a  |    /              ^  parent    
 *  p  |   V                 \ 
 *     -----------   next    ------------  next
 *    | childHead | ------> | nextChild  |----> .....
 *     -----------           ------------
 */
typedef struct SMemoryContainer
{
	MemContType		            type;			
	struct SMemoryContainer*    parent;	

	/* Every node can have children and they are 
	 * organized into a linked list. 
	 * Current node has a link only to a head of
	 * the children linked list.
	 */
    struct SMemoryContainer*    childHead;

    /* Also the node is an element in a children linked list.
	 * And it has a reference to the next neighbour.
	 */
    struct SMemoryContainer*    next;
	char*                       name;			/* container name  */
	Bool		                isReset;		
} SMemoryContainer, *MemoryContainer;




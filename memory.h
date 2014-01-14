

/* The smallest chunk of memory is 8 bytes.
 * 8 = 2^3. So that the smallest power of 2 is 3.
 */
#define MIN_CHUNK_POWER_OF_2	    3	
#define MEMORY_SET_FREELISTS_NUM	11

/* The maximum allowed memory chunk size
 */
#define MEMORY_CHUNK_MAX_SIZE  (1 << (MEMORY_SET_FREELISTS_NUM-1+MIN_CHUNK_POWER_OF_2))

typedef enum MemContType
{
   MCT_Invalid         = 0,
   MCT_MemoryContainer = 10
} MemContType;

#define MEM_BLOCK_SIZE	   ALIGN_DEFAULT(sizeof(SMemoryBlock))
#define MEM_CHUNK_SIZE	   ALIGN_DEFAULT(sizeof(SMemoryChunk))

#define UNUSED_SPACE_MARK  0x7E  /* In dec it is 126, in binary - 0111 1110 */

#define MemoryChunkGetPointer(chunk)	\
      ((void*)(((char*)(chunk)) + MEM_CHUNK_SIZE))

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

typedef struct SMemoryBlock* MemoryBlock;
typedef struct SMemoryChunk* MemoryChunk;

typedef struct SMemorySet
{
	/* SetContainer inherits from MemoryContainer 
	 * So that the first structure field should be 
	 * a reference to the base class.
	 */
	SMemoryContainer   baseMemCont;	

	/* head of list of blocks in the memory set */
	MemoryBlock	       blockList;	

	/* Lists of free memory chunks 
	 * Each chunk free list contains pieces of memory
	 * of a defunite size and this size is a power of 2
	 */
	MemoryChunk	       freelist[MEMORY_SET_FREELISTS_NUM];	

	/* initial block size */
	size_t		       initBlockSize;	

    /* maximum block size */
	size_t		       maxBlockSize;	

    /* next block size to allocate */
	size_t		       nextBlockSize;	

    /* effective chunk size limit */
	size_t		       chunkMaxSize;
	MemoryBlock	       keeperBlock;	
} SMemorySet, *MemorySet;


typedef struct SMemoryBlock
{
	MemorySet	  memset;		  /* MemorySet which the block belongs to */
	MemoryBlock	  next;			  /* next block in the memory set's blocks list */
	char*         freeStart;	  /* start of free space in this block */
	char*         freeEnd;		  /* end of free space in this block */
} SMemoryBlock;


typedef struct SMemoryChunk
{
	/* If the chunk is inside a free list it points to 
	 * the next chunk in the free list. If this chunk 
	 * is withdrawn from the free list it points to 
	 * the parent set.
	 */
	void*         memsetorchunk;

	/* The size of memory which is actually used 
	 * in the chunk.
	 */
	size_t		  size;

	/* The size of memory which has been actually 
	 * requested by a user.
	 */
	size_t		  sizeRequested;
} SMemoryChunk;



/* If we are requesting for a memory of size x
 * where 0 <= x <= 256.  256 = 2^8.
 * We would like only to allocate memory chunks 
 * that are of power of 2 size. We have 9 possible values:
 * 2^0, 2^1, 2^2, 2^3, 2^4, 2^5, 2^6, 2^7, 2^8
 * 1,   2,   4,   8,   16,  32,  64,  128, 256
 * 
 * If we request some size of memory which is not a power of 2,
 * we should round it up to the nearest larger power of 2.
 * For example if we request 45. we should allocate 64.
 */
#define SeqOf16CopiesOf(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n

/* This table matches n to log2(n) + 1 
 * It has been created to avoid calculating it
 * all time.
 */
unsigned char Log2Table[256] =
{
	0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
	SeqOf16CopiesOf(5), SeqOf16CopiesOf(6), SeqOf16CopiesOf(6), SeqOf16CopiesOf(7), 
	SeqOf16CopiesOf(7), SeqOf16CopiesOf(7), SeqOf16CopiesOf(7),
	SeqOf16CopiesOf(8), SeqOf16CopiesOf(8), SeqOf16CopiesOf(8), SeqOf16CopiesOf(8), 
	SeqOf16CopiesOf(8), SeqOf16CopiesOf(8), SeqOf16CopiesOf(8), SeqOf16CopiesOf(8)
};





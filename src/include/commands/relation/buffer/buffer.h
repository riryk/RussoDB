
#ifndef BUFFER_H
#define BUFFER_H

#include "common.h"
#include "relfile.h"
#include "latch.h"

/* Number of partitions of the shared buffer mapping hashtable 
 * Partitions in a hashtable has been created to reduce contention
 * which is caused by one single lock.
 * We split a hashtable to parts and can simultaneously be processed.
 */
#define BUFFER_HASHTABLE_PARTITIONS  16

/* Buffers and blocks are the same.
 * It consists of relation identifier,
 * relation part and block number.
 * This is the physical disk buffer identifier.
 */
typedef struct SBufferId
{
	SRelFileInfo    relId;
	FilePartNumber	relPart;
	uint            blockNum;
} SBufferId, *BufferId;

/* The structure which is used in the buffer cache hash table.
 * The first field bufId is the hashtable key.
 * The second field is the hashtable value
 * and actually is an buffer's identifier in an array in memory.
 * We do not keep a buffer as a value but have a separate array.
 */
typedef struct SBufCacheItem
{
    SBufferId       bufId;     /* Disk buffer id */
    int             id;        /* id in an array in memory */
} SBufCacheItem, *BufCacheItem;

typedef struct SBufferInfo
{
	SBufferId	 id;			
	uint16	     flags;			

	/* If usageCount = 0, the buffer is not being used.
	 * If usageCount > 0, the buffer is being used. */
	uint16		 usageCount;  

    /* Number of backend processes 
	 * which are holding the buffer's pin */	
	uint	     refCount;	  
	int			 backWaiterId;

	/* The buffer's index in Buffer's array */
	int			 ind;

	/* Link to the the next free item from the free list. */
	int 		 freeNext;	
} SBufferInfo, *BufferInfo;


#define FREENEXT_END_OF_LIST	(-1)
#define FREENEXT_NOT_IN_LIST	(-2)


typedef enum BufRingAccessType
{
	BAS_NORMAL,					/* Normal random access */
	BAS_BULKREAD,				/* Large read-only scan (hint bit updates are ok) */
	BAS_BULKWRITE,				/* Large multi-block write (e.g. COPY IN) */
	BAS_VACUUM					/* VACUUM */
} BufRingAccessType;

typedef struct SBufRing
{
	/* Buffer ring access type */
	BufRingAccessType    type;

	/* Number of elements in buffers array 
	 * which represents the ring 
	 */
	int                  size;

	/* Index of "current" buffer in the buffer array.
	 * It is the last most recent returned victim buffer.
	 */
	int	                 current;

	/* True if the current had been in the ring already. */
	Bool                 currentInRing;

	/* Array of ring buffer numbers. 
	 * If buffers[i] == -1 that means we have not
	 * selected this buffer as a victim.
	 */
	int		             buffers[1];		
} SBufRing, *BufRing;


typedef struct SBufFreeListState
{
	/* Next buffer to take as a victim buffer */
	int			next;

	/* Head of list of unused buffers */
	int			first;	

	/* Tail of list of unused buffers */
	int			last;     

	/* Statistic information
	 * Completed cycles of the clock sweep algorithm */
	uint		completedCycles;     

	/* Statistic information
	 * allocated buffers. */
	uint		bufferAllocated;

	/* Notification latch.
	 * It is null if noone is waiting for notification
	 */
	Latch	    latch;
} SBufFreeListState, *BufFreeListState;


#define MAX_USAGE_COUNT 5

#define BUFFER_DIRTY			  (1 << 0)		/* needs writing */
#define BUFFER_VALID			  (1 << 1)		
#define BUFFER_ID_VALID			  (1 << 2)		
#define BUFFER_IO_IN_PROGRESS     (1 << 3)		/* file read or write in progress */
#define BUFFER_IO_ERROR			  (1 << 4)		/* file I/O failed */
#define BUFFER_DIRTIED			  (1 << 5)		/* was dirtied */
#define BUFFER_PIN_WAITER		  (1 << 6)		/* there is a thread which is waiting for pin releasing */
#define BUFFER_CHECKPOINT_NEEDED  (1 << 7)		
#define BUFFER_PERMANENT		  (1 << 8)		/* permanent relation */
#define BUFFER_JUST_DIRTIED       (1 << 9)    


#define BUFFER_ID_DEFAULT(b) \
( \
	(b).relId.tblSpaceId = (-1), \
	(b).relId.databaseId = (-1), \
	(b).relId.relId = (-1), \
	(b).relPart = (-1), \
	(b).blockNum = (-1) \
)

#endif
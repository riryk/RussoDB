
#ifndef BUFFER_H
#define BUFFER_H

#include "common.h"
#include "relfile.h"
#include "latch.h"

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
	SBufferId	 bufId;			
	uint16	     flags;			

	/* If usageCount = 0, the buffer is not being used.
	 * If usageCount > 0, the buffer is being used. */
	uint16		 usageCount;  

    /* Number of backend processes 
	 * which are holding the buffer's pin */	
	uint	     refCount;	  
	int			 backWaiterId;
	int			 bufInd;
	int			 freeNext;	
} SBufferInfo, *BufferInfo;


typedef enum BufRingAccessType
{
	BAS_NORMAL,					/* Normal random access */
	BAS_BULKREAD,				/* Large read-only scan (hint bit updates are ok) */
	BAS_BULKWRITE,				/* Large multi-block write (e.g. COPY IN) */
	BAS_VACUUM					/* VACUUM */
} BufferAccessStrategyType;

typedef struct SBufRing
{
	/* Buffer ring access type */
	BufRingAccessType    atype;

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

	/* Array of ring buffer numbers. */
	int		             buffers[1];		
} SBufRing, *BufRing;


typedef struct SBufferStrategyControl
{
	/* Clock sweep hand: index of next buffer to consider grabbing */
	int			nextVictimBuffer;

	int			firstFreeBuffer;	/* Head of list of unused buffers */
	int			lastFreeBuffer;     /* Tail of list of unused buffers */

	/*
	 * NOTE: lastFreeBuffer is undefined when firstFreeBuffer is -1 (that is,
	 * when the list is empty)
	 */

	/*
	 * Statistics.	These counters should be wide enough that they can't
	 * overflow during a single bgwriter cycle.
	 */
	uint		completePasses;     /* Complete cycles of the clock sweep */
	uint		numBufferAllocs;	/* Buffers allocated since last reset */

	/*
	 * Notification latch, or NULL if none.  See StrategyNotifyBgWriter.
	 */
	Latch	   bgwriterLatch;
} SBufferStrategyControl, *BufferStrategyControl;


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

#endif
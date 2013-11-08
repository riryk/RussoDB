
#ifndef BUFFER_H
#define BUFFER_H

#include "common.h"


typedef struct SBufferId
{
	SRelFileInfo    relId;
	FilePartNumber	relPart;
	uint32          blockNum;
} SBufferId, *BufferId;

typedef struct SBufCacheItem
{
    SBufferId       bufId;
    int             id;
} SBufCacheItem, *BufCacheItem;

typedef struct SBufferInfo
{
	SBufferId	 bufId;			
	uint16	     flags;			
	uint16		 usageCount;  /* usage counter */
	uint	     refCount;	  /* number of backend processes 
							   * which are holding the buffer's pin */	
	int			 backWaiterId;
	int			 bufInd;
	int			 freeNext;	
} SBufferInfo, *BufferInfo;

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

#endif
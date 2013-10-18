
#include <io.h>
#include "common.h"

#ifndef NEW_H
#define NEW_H


#define TRAN_LOG_BLOCK_SIZE  (1024 * 64)
#define TRAN_LOG_SEG_SIZE    (1024 * 64 * 8)
#define TRAN_LOG_SEG_NUMBER	 (0x100000000 / TRAN_LOG_SEG_SIZE)

#define TRAN_LOG_FILES_COUNT (2*3 + 1)

typedef unsigned long long int uint64;

/*typedef struct BlockId
{
	unsigned short		high;
	unsigned short		low;
} BlockId;*/

struct tran_log_insert
{
	unsigned int		tableSpaceId;		/* tablespace */
	unsigned int 		databaseId;		    /* database */
	unsigned int		relationId;		    /* relation */

	BlockId             blockId;
	unsigned short      position;
	unsigned short		allCleared;	
};

struct tran_log_header
{
    unsigned short		mask2;
	unsigned short		mask;
	unsigned char		offset;
};

struct tran_log_page_header
{
    unsigned short		magic;	 
};

struct tran_log_data
{
	char*                 data;			
	unsigned int		  length;
	int		              bufferId;	
	int		              bufferStandard;
	struct tran_log_data* next;	
};

struct tran_log_record
{
	unsigned int		  totalLength;	
	unsigned int          tranId;
	unsigned int		  length;
	unsigned char		  info;
	uint64	              prev;	
};

struct tran_log_id
{
	unsigned int		  log_id;	
	unsigned int		  tran_offset;
};

struct RelationFileInfo
{
	unsigned int		tableSpaceId;		/* tablespace */
	unsigned int 		databaseId;		    /* database */
	unsigned int		relationId;		    /* relation */
};

typedef enum Fork
{
	INVALID_FORK = -1,
	MAIN_FORK = 0,
	FSM_FORK,
	VISIBILITYMAP_FORK,
	INIT_FORK
} Fork;

struct BackedUpBlock
{
	struct RelationFileInfo      node;			
	Fork	              fork;			
	unsigned int          block;
	unsigned short		  gapOffset;
	unsigned short		  gapLength;
};

struct PageItemData
{
	unsigned int	offsetToRow:15;
	unsigned int    state:2;	
	unsigned int	length:15;	
};

struct PageHeader
{
	unsigned int   tranLogId;
	unsigned int   tranRecordOffset;

    unsigned short startFreeSpace;
	unsigned short endFreeSpace;
	unsigned short startSpecialFreeSpace;
	struct PageItemData   rowInfoData[1];	
};

struct TranLogInsertState
{
    char*                currentPosition;		
	struct tran_log_page_header currentPage;
	int			         currentBlockIndex;	
	struct tran_log_id*         blocks; 
};

struct TranLogProgress
{
	struct tran_log_id	  WritePosition;
	struct tran_log_id	  FlushPosition;
};

struct TranLogWriteProgress
{
	struct tran_log_id	  WritePosition;
	struct tran_log_id	  FlushPosition;
};

struct TranLogWriteState
{
	int	   currentIndex;			
};

struct TranLogState
{
    struct TranLogInsertState InsertState;
	int		                  highestBlock;
	long                      locker;
    struct TranLogProgress*   LogProgress;  
	struct TranLogWriteState  WriteState; 
	struct tran_log_id*              blocks; 
	char*                     pages;
};


#endif

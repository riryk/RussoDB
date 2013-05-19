
typedef unsigned long long int uint64;

typedef struct BlockId
{
	unsigned short		high;
	unsigned short		low;
} BlockId;

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


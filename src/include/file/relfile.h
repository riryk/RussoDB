
#define DEFAULT_TBL_SPACE 1663
#define GLOBAL_TBL_SPACE 1664

#define INVALID_BACK_ID (-1)

typedef struct SRelFileInfo
{
	uint		  tblSpaceId;
	uint 		  databaseId;
	uint		  relId;		
} SRelFileInfo, *RelFileInfo;

typedef struct SRelFileInfoBack
{
	SRelFileInfo  node;
	int	          backend;
} SRelFileInfoBack, *RelFileInfoBack;

typedef struct SFileSeg
{
	int		          fileDesc;
	uint              segNum;
	struct SFileSeg*  segNext
} SFileSeg, *FileSeg;

typedef enum FilePartNumber
{
	FilePartInvalid          = -1,
	FILE_PART_MAIN           = 0,
	FILE_PART_FILE_STORAGE,
	FILE_PART_VISIBILITY_MAP,
	FILE_PART_INIT
} FilePartNumber;

const char* filePartNames[] = 
{
	"main",						/* FILE_PART_MAIN */
	"fs",						/* FILE_PART_FILE_STORAGE */
	"vm",						/* FILE_PART_VISIBILITY_MAP */
	"init"						/* FILE_PART_INIT */
};

#define FILE_PART_MAX FILE_PART_INIT

typedef struct SRelData
{ 
	int         currentBlock;
	FileSeg     parts[FILE_PART_MAX + 1];
} SRelData, *RelData;


/* First argument is a RelFileNodeBackend */
#define relpath(rnode, forknum) \
		relpathbackend((rnode).node, (rnode).backend, (forknum))


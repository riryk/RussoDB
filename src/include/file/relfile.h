
#ifndef REL_FILE_H
#define REL_FILE_H


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
	/* File's index in the file cache.
	 * Not file descriptor.
	 */
	int		          find;
	uint              num;
	char*             fname;
	struct SFileSeg*  next;
} SFileSeg, *FileSeg;

typedef FileSeg *AFileSeg;

typedef enum FilePartNumber
{
	FilePartInvalid          = -1,
	FILE_PART_MAIN           = 0,
	FILE_PART_FILE_STORAGE,
	FILE_PART_VISIBILITY_MAP,
	FILE_PART_INIT
} FilePartNumber;

typedef enum ExtensionBehavior 
{
	EXTENSION_FAIL,			
	EXTENSION_RETURN_NULL,	
	EXTENSION_CREATE		
} ExtensionBehavior;

#define FILE_PART_MAX FILE_PART_INIT

typedef struct SRelData
{ 
    SRelFileInfoBack  relKey;
	int               currentBlock;
	AFileSeg          parts[FILE_PART_MAX + 1];
} SRelData, *RelData;

#endif





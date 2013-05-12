
#include "relation_storage.h"

int	   SimultaneousProcessMode = 0;
int    FlushToDiskProcess      = 0;
int    StartupProcess          = 0;

static int SyncNumber          = 0;
static int SyncInProgress      = 0;

static int EnableFsync         = 1;
static int InRecovery          = 0;


typedef struct RelationSegment
{
	int		                  fileDescriptor;		
	unsigned int              segmentNumber;
	struct RelationSegment*   segmentNext;	
} RelationSegment;

typedef struct StorageRelation
{
   RelationFileBackend  relationKey;
   struct StorageRelation** Parent;
   int CurrentBlock;
   int FsmForkSize;
   int VmForkSize;
   int StorageManager;
   RelationSegment* Segments[INIT_FORK + 1];
   StorageRelation* NextRelation;
} StorageRelation;

typedef struct RelationFileInfo
{
	unsigned int		tableSpaceId;		/* tablespace */
	unsigned int 		databaseId;		    /* database */
	unsigned int		relationId;		    /* relation */
} RelationFileInfo;

typedef struct RelationFileBackend
{
	RelationFileInfo   fileInfo;
	int	               backend;
} RelationFileBackend;

typedef struct
{
	RelationFileInfo    Key;	                      /* hash table key */
	int             	SyncNumber;		
	Set*    FSyncRequests[MAX_REL_PARTS_NUMBER + 1];  /* fsync requests */
} FSyncRequestItem;

typedef struct StorageRelationData
{
	RelationFileInfo    Key;
	/* hash table key */
} StorageRelationData;

typedef enum					
{
	FAIL,				        
	RETURN_NULL,		        
	CREATE			            
} Behavior;

static HashTable *requestsTable = NULL;
static HashTable *storageRelationTable = NULL;

void RelationStorageInit()
{
	if (!SimultaneousProcessMode || FlushToDiskProcess || StartupProcess)
	{
		HashTableSettings		hashTableSett;

		MemSet(&hashTableSett, 0, sizeof(hashTableSett));

		hashTableSett.KeyLength = sizeof(RelationFileInfo);
		hashTableSett.ValueLength = sizeof(FSyncRequestItem);
        hashTableSett.HashFunc = HashSimple;

		requestsTable = HashTableCreate("Requests Table",
									    100L,
									    &hashTableSett,
								        HASH_FUNCTION);
	}
}

StorageRelation RelationOpen(RelationFileInfo fileInfo, int backend)
{
	RelationFileBackend  fileBackend; 
	StorageRelation      relation;
	int		             found;
	int                  i;

	if (storageRelationTable == NULL)
	{
		HashTableSettings		hashTableSett;

		MemSet(&hashTableSett, 0, sizeof(hashTableSett));

		hashTableSett.KeyLength = sizeof(RelationFileInfo);
		hashTableSett.ValueLength = sizeof(StorageRelationData);
        hashTableSett.HashFunc = TagHash;

		storageRelationTable = HashTableCreate("Storage Relation Table",
									    400,
									    &hashTableSett,
								        HASH_FUNCTION);
	}
	
	fileBackend.fileInfo = fileInfo;
	fileBackend.backend = backend;

	relation = HashSearch(storageRelationTable, 
		                  (void*)&fileBackend,
		                  HASH_ENTER,
		                  &found);

	/* If this item exists in the hash table found will be true */
	if (!found)
	{
		int			forknum;

		relation->Parent = NULL;

		relation->CurrentBlock = -1;
		relation->FsmForkSize = -1;
		relation->VmForkSize = -1;
		relation->StorageManager = 0;	

		for (i = 0; i <= INIT_FORK; i++)
			relation->Segments[i] = NULL;
	}

	return relation;
}

static RelationSegment* OpenSegment(StorageRelation relation, int forkNumber)
{
    RelationSegment* segment;
	char* filePath;
	int   fileDescriptor;

	if (relation->Segments[forkNumber])
		return relation->Segments[forkNumber];
    
	filePath = GenerateFilePath(relation->relationKey, relation->relationKey->backend, forkNumber);
	fileDescriptor = ROpenFile(filePath, O_RDWR | PG_BINARY, 0600);

	if (fileDescriptor < 0)
		Error("Could not open file %s", filePath);

    free(filePath);

    segment = (RelationSegment*)malloc(sizeof(RelationSegment));
	relation->Segments[forkNumber] = segment;

    segment->fileDescriptor = fileDescriptor;
	segment->segmentNumber = 0;
	segment->segmentNext = NULL;

	return segment;
}



static int GetNumberOfBlocls(StorageRelation relation, int forkNumber, RelationSegment*  segment)
{
    long length;
	length = FileSeek(segment->fileDescriptor, 0, SEEK_END);
	if (length < 0)
       Error("Could not seek in file");
    return length / BLOCK_SIZE;
}

static RelationSegment* GetSegment(
	StorageRelation relation, 
	int forkNumber, 
	int blockNumber,
	int skipFsync, 
	Behavior behavior)
{
	int i;
    RelationSegment*    segment = OpenSegment(relation, forkNumber);  
    int segmentNum = blockNumber / RELATION_SEGMENT_SIZE;

	for (i = 1; i <= segmentNum; i++)
	{
       if (segment->segmentNext == NULL)
	   {
           if (behavior == CREATE || InRecovery)
		   {
               if (GetNumberOfBlocls(relation, forkNumber, segment) < RELATION_SEGMENT_SIZE)
			   {
                   char* newBuffer = malloc(BLOCK_SIZE);
                   long  seekPosition;
				   int   bytesWritten;

                   MemSet(newBuffer, 0, BLOCK_SIZE);

				   if (blockNumber == 1 << 32 - 1)
	                   Error("Could not extend");
                   
                   seekPosition = (long)BLOCK_SIZE*(blockNumber % RELATION_SEGMENT_SIZE);
				   if (FileSeek(segment, seekPosition, seekPosition) != seekPosition)
                       Error("Could not seek into block");

				   if ((bytesWritten = FileWrite(segment, buffer, BLOCK_SIZE)) != BLOCK_SIZE)
					   Error("Could not write into block");
                   
                   free(newBuffer); 
			   }
			   segment->segmentNext = OpenSegment(relation, forknum);
		   }
		   else
		   {
               segment->segmentNext = OpenSegment(relation, forknum);
		   }
           if (segment->segmentNext == NULL)
               Error("Could not open file");
	   }
	   segment = segment->segmentNext;
	}
	return segment;
}

void RelationWritesSync()
{
	HashSequenceItem  hashSequence;
	FSyncRequestItem* request;

	HashSequenceInit(&hashSequence, &requestsTable);
	while ((request = (FSyncRequestItem*)HashSequenceSearch(&hashSequence)) != NULL)
	{
		request->SyncNumber = SyncNumber;
	}

	SyncNumber++;
	SyncInProgress = 1;

	HashSequenceInit(&hashSequence, &requestsTable);
	while ((request = (FSyncRequestItem*)HashSequenceSearch(&hashSequence)) != NULL)
	{
		Fork	fork;

		if (request->SyncNumber == SyncNumber)
			continue;

		for (fork = 0; fork <= INIT_FORK; fork++)
		{
			Set*        requests = request->FSyncRequests[fork];
			int			segmentNumber;

			request->FSyncRequestsv[fork] = NULL;

			while ((segmentNumber = Set_GetFirstMember(requests)) >= 0)
			{
				int			failures;

				if (!EnableFsync)
					continue;

				for (failures = 0;;failures++) 
				{
					StorageRelation   storageRel;
					RelationSegment*  segment;

					storageRel = RelationOpen(request->Key, -1);
					segment = GetSegment(storageRel, 
						       fork, 
							   segmentNumber * RELATION_SEGMENT_SIZE,
	                           0, 
	                           RETURN_NULL);

					if (segment != NULL && FileSync(segment->fileDescriptor) >= 0)
						break;	
				}	
			}
			free(requests);
		}
	}	

	SyncInProgress = 0;
}

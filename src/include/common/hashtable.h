#include <stdio.h>
#include <io.h>
#include <string.h>
#include "error.h"
#include "common.h"

#define MOD(x,y)			   ((x) & ((y)-1))

typedef unsigned int (*HashFunction) (void* Key, unsigned long KeySize);

typedef int (*HashCompareFunc) (void* Key1, void* Key2, unsigned long KeySize);

typedef void* (*HashCopyFunc) (void* Destination, void* Source, unsigned long KeySize);

typedef void* (*HashAllocFunc) (unsigned int Size);

typedef enum
{
	FIND,
	INSERT,
	DELETE,
	INSERT_NULL
} EHashAction;

typedef struct SHashtableSettings
{
	long             PartNumber;      /* Number of sets on which the whole data will be divided */
	uint	         KeyLength;       /* hash key length in bytes */
    uint             ValueLength;     /* hash table value size in bytes */
	long		     SegmentSize;			
	int			     SegmentShift;	
	HashFunction     HashFunc;
	HashCompareFunc  HashCompare;
	HashCopyFunc     HashCopy;		 
} SHashtableSettings, *HashtableSettings;

typedef struct SHashtable
{
	HashFunction            HashFunc;			    /* hash function */
	HashCompareFunc         HashCompare;
	HashCopyFunc            HashCopy;		
	HashAllocFunc           HashAlloc;
	char*                   Name;		            /* name */
	int		                IsInSharedMemory;		/* is in shared memory */
	int		                ProhibitEnlarge;  		
	int		                ProhibitInserts;	    
	uint	                KeyLength;		    
	long		            SegmentSize;			
	int			            SegmentShift;			
    struct HashtableHeader* Header;
	struct HashItem**       StartSegment;		
	int                     PartitionNumber;
	struct HashItem***      Directory;
} SHashtable, Hashtable;

struct HashtableHeader
{
	long             ItemsNumber;
    long             MaxBucketId;
    long             BucketSize;
	unsigned int     SegmentsCount;
	unsigned int     DirectorySize;
	long             MaxDirectorySize;
	unsigned int	 HighMask;		
	unsigned int	 LowMask;		
    long             Locker;
	struct HashItem*        FreeList;
	int              DataItemSize;
	int              ItemsNumToAllocAtOnce;
};

#define HASH_FUNCTION	0x010	
#define HASH_COMPARE	0x400	
#define HASH_KEYCOPY	0x800	
#define HASH_ALLOC		0x100

struct HashItem
{
	struct HashItem*    Next;	    
	unsigned int		Hash;		
};

struct HashSequenceItem
{
	struct Hashtable*      Table;
	unsigned int	CurrentBucket;
	struct HashItem*       CurrentItem;		
};


#define SEQUENCE_MAX_SCANS 100

static struct Hashtable* SequenceScans[SEQUENCE_MAX_SCANS];
static int SequenceScansCount = 0;

unsigned int HashSimple(void* Key, unsigned long KeySize);
unsigned int HashForRelId(void* Key, unsigned long KeySize);

int StringCmp(char* Key1, char* Key2, unsigned long KeySize);

struct HashTable* HashTableCreate(
	char* Name, 
	long MaxItemsNum, 
	struct HashTableSettings* Info, 
	int Flags);
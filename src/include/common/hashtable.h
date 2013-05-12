#include <stdio.h>
#include <io.h>
#include <string.h>
#include "error.h"

#define MOD(x,y)			   ((x) & ((y)-1))

typedef unsigned int (*HashFunction) (void* Key, DWORD KeySize);

typedef int (*HashCompareFunc) (void* Key1, void* Key2, DWORD KeySize);

typedef void* (*HashCopyFunc) (void* Destination, void* Source, DWORD KeySize);

typedef void* (*HashAllocFunc) (unsigned int Size);

typedef enum
{
	FIND,
	INSERT,
	DELETE,
	INSERT_NULL
} EHashAction;

typedef struct HashTableSettings
{
	long             PartNumber;      /* Number of sets on which the whole data will be divided */
	unsigned int	 KeyLength;       /* hash key length in bytes */
    unsigned int     ValueLength;     /* hash table value size in bytes */
	long		     SegmentSize;			
	int			     SegmentShift;	
	HashFunction     HashFunc;
	HashCompareFunc  HashCompare;
	HashCopyFunc     HashCopy;		 
} HashTableSettings;


typedef struct Hashtable
{
	HashValueFunc    HashFunc;			/* hash function */
	HashCompareFunc  HashCompare;
	HashCopyFunc     HashCopy;		
	HashAllocFunc    HashAlloc;
	char*            Name;		        /* name */
	int		         IsInShared;		/* is in shared memory */
	int		         NotEnlarge;  		
	int		         NoMoreInserts;	    
	unsigned int	 KeyLength;		    
	long		     SegmentSize;			
	int			     SegmentShift;			
    HashtableHeader* Header;
	HashItem**       StartSegment;		
	int              PartitionNumber;
	HashItem***      Directory;
} Hashtable;

typedef struct HashtableHeader
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
	HashItem*        FreeList;
	int              DataItemSize;
	int              ItemsNumToAllocAtOnce;
} HashtableHeader;

#define HASH_FUNCTION	0x010	
#define HASH_COMPARE	0x400	
#define HASH_KEYCOPY	0x800	
#define HASH_ALLOC		0x100

typedef struct HashItem
{
	struct HashItem*    Next;	    
	unsigned int		Hash;		
} HashItem;

typedef struct
{
	Hashtable*      Table;
	unsigned int	CurrentBucket;
	HashItem*       CurrentItem;		
} HashSequenceItem;


#define SEQUENCE_MAX_SCANS 100

static Hashtable* SequenceScans[SEQUENCE_MAX_SCANS];
static int SequenceScansCount = 0;

HashTable* HashTableCreate(
	char* Name, 
	long MaxItemsNum, 
	HashTableSettings* Info, 
	int Flags);
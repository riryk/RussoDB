#include <stdio.h>
#include <io.h>
#include <string.h>
#include "error.h"
#include "memorymanager.h"
#include "hashfunctions.h"

typedef enum
{
	FIND,
	INSERT,
	DELETE,
	INSERT_NULL
} EHashAction;

typedef struct SHashtableSettings
{
	ulong            partNum;      /* Number of sets on which the whole data will be divided */
	uint	         keyLen;       /* hash key length in bytes */
    uint             valLen;       /* hash table value size in bytes */
	ulong		     segmSize;			
	uint			 segmShift;	
	hashFunc         hashFunc;
	hashCmpFunc      hashCmp;
	hashCpyFunc      hashCpy;		 
} SHashtableSettings, *HashtableSettings;

typedef struct SHashtable
{
	char*                   name;		            /* name */
	Bool		            isInShared;		/* is in shared memory */
	Bool		            noEnlarge;  		
	Bool		            noInserts;	    
	uint	                keyLen;		    
	ulong		            segmSize;			
	uint			        segmShift;			
    struct HashtableHeader* header;
	struct HashItem**       startSegm;		
	uint                    partNum;
	struct HashItem***      dir;
    hashFunc                hashFunc;			    /* hash function */
	hashCmpFunc             hashCmp;
	hashCpyFunc             hashCpy;		
	hashAllocFunc           hashAlloc;
} SHashtable, *Hashtable;

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

#define HASH_FUNC	    0x001
#define HASH_CMP	    0x002
#define HASH_SEG	    0x004
#define HASH_KEYCPY	    0x008	
#define HASH_ALLOC		0x010

struct HashItem
{
	struct HashItem*    Next;	    
	unsigned int		Hash;		
};

struct HashSequenceItem
{
	Hashtable              Table;
	unsigned int	       CurrentBucket;
	struct HashItem*       CurrentItem;		
};


#define SEQUENCE_MAX_SCANS 100

static struct Hashtable* SequenceScans[SEQUENCE_MAX_SCANS];
static int SequenceScansCount = 0;

unsigned int HashSimple(void* Key, unsigned long KeySize);
unsigned int HashForRelId(void* Key, unsigned long KeySize);

int StringCmp(char* Key1, char* Key2, unsigned long KeySize);

 Hashtable createHashtable(
	char*              name, 
	long               maxItemsNum, 
	HashtableSettings  set, 
	int                setFlags);
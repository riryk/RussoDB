#include <stdio.h>
#include <io.h>
#include <string.h>
#include "error.h"
#include "memorymanager.h"
#include "hashfunctions.h"


#define SEQUENCE_MAX_SCANS           100
#define DEFAULT_SEG_SIZE             256
#define DEFAULT_SEG_SHIFT            8
#define DEFAULT_HASH_LIST_SIZE       4
#define DEFAULT_SEGS_AMOUNT          256
#define NO_SEGS_AMOUNT_RESTRICTIONS  (-1)

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
	uint             hashListSize;
	uint             segmsAmount;
    uint             maxSegmsAmount;
	hashFunc         hashFunc;
	hashCmpFunc      hashCmp;
	hashCpyFunc      hashCpy;		 
} SHashtableSettings, *HashtableSettings;


typedef struct SHashItem
{
	HashItem            next;	    
	uint		        hash;		
} SHashItem, *HashItem;

/* A name convention: if some struct has a link to another the same struct,
 * we have a linked link. HashItem in this case is also a pointer to a head 
 * of a linked list. All linked list start with LL prefix */
typedef HashItem   LLHashItem;
typedef LLHashItem HashList; /* A linked list of hashitems */

/* Sometimes it is very difficult to distinguish whether a pointer to some type
 * is just a pointer or an array. All arrays start with 'A' prefix */
typedef HashList*  AHashList;
typedef AHashList  HashSegment; /* Hash segment is an array of HashLists */

typedef HashSegment* AHashSegment; /* A array of HashSegments */

typedef struct SHashtable
{
	char*                   name;		    /* name */
	Bool		            isInShared;		/* is in shared memory */
	Bool		            noEnlarge;  		
	Bool		            noInserts;	    
	uint	                keyLen;		    

    /* Each hashtable consists of an array os segments.
	 * Each segment is a list of a hash items linked lists. */
    AHashSegment            segments;	    
	ulong		            segmSize;			
	uint			        segmShift;	
	uint                    segmsAmount;
    uint                    maxSegmsAmount;
	uint                    nSegs;

	uint                    hashListSize;
    struct HashtableHeader* header;
	struct HashItem**       startSegm;		
	uint                    partNum;
    hashFunc                hashFunc;	    /* hash function */
	hashCmpFunc             hashCmp;
	hashCpyFunc             hashCpy;		
	hashAllocFunc           hashAlloc;
	uint32		            highMask;		
	uint32		            lowMask;		
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
	struct HashItem* FreeList;
	int              DataItemSize;
	int              ItemsNumToAllocAtOnce;
};

#define HASH_FUNC	    0x001
#define HASH_CMP	    0x002
#define HASH_SEG	    0x004
#define HASH_KEYCPY	    0x008	
#define HASH_ALLOC		0x010
#define HASH_LIST_SIZE  0x020

struct HashSequenceItem
{
	Hashtable              Table;
	unsigned int	       CurrentBucket;
	struct HashItem*       CurrentItem;		
};

static struct Hashtable* SequenceScans[SEQUENCE_MAX_SCANS];
static int SequenceScansCount = 0;

Hashtable createHashtable(
	char*              name, 
	long               maxItemsNum, 
	HashtableSettings  set, 
	int                setFlags);
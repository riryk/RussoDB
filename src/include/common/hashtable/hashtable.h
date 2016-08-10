#include <stdio.h>
#include <io.h>
#include <string.h>
#include "error.h"
#include "imemorymanager.h"
#include "hashfunctions.h"
#include "semaphore.h"

#ifndef HASHTABLE_H
#define HASHTABLE_H


#define SEQUENCE_MAX_SCANS           100
#define DEFAULT_SEG_SIZE             256
#define DEFAULT_SEG_SHIFT            8
#define DEFAULT_HASH_LIST_SIZE       4
#define DEFAULT_SEGS_AMOUNT          256
#define NO_SEGS_AMOUNT_RESTRICTIONS  (-1)

typedef enum
{
	HASH_FIND,
	HASH_INSERT,
	HASH_DELETE,
	HASH_INSERT_NULL
} EHashAction;

typedef struct SHashtableSettings
{
	ulong            partNum;       /* Number of sets on which the whole data will be divided 
									 * Number of max parallel processes which can work with a table */
	uint	         keyLen;        /* hash key length in bytes */
    uint             valLen;        /* hash table value size in bytes */
	ulong		     segmSize;			
	uint			 segmShift;	
	uint             hashListSize;
	uint             segmsAmount;
    uint             maxSegmsAmount;
	hashFunc         hashFunc;
	hashCmpFunc      hashCmp;
	hashCpyFunc      hashCpy;		 
	Bool             isWithoutExtention;
} SHashtableSettings, *HashtableSettings;

typedef struct SHashItem
{
	struct SHashItem*   next;	    
	uint		        hash;
} SHashItem, *HashItem;

/* A name convention: if some struct has a link to another the same struct,
 * we have a linked link. HashItem in this case is also a pointer to a head 
 * of a linked list. All linked list start with LL prefix */
typedef HashItem   LLHashItem;
typedef LLHashItem HashList; /* A linked list of hashitems */

/* Sometimes it is very difficult to distinguish whether a pointer to some type
 * is just a pointer or an array. All arrays start with 'A' prefix */
typedef HashList  *AHashList;
typedef AHashList  HashSegment; /* Hash segment is an array of HashLists */

typedef HashSegment* AHashSegment; /* A array of HashSegments */

typedef struct SHashtable
{
	Semaphore               mutex;
	Bool		            isInShared;		/* is in shared memory */
	Bool		            noEnlarge;  		
	Bool		            noInserts;	    
	uint	                keyLen;		    
    uint                    valLen;
	char*                   name;		    /* name */
    /* Each hashtable consists of an array os segments.
	 * Each segment is a list of a hash items linked lists. */
    AHashSegment            segments;	    
	ulong		            segmSize;			
	uint			        segmShift;	
	/* This is a total number of segments in the hash table */
	uint                    segmsAmount;
    int                     maxSegmsAmount;
	/* This is a current number of allocated segments */
	uint                    nSegs;
	uint                    hashListSize;
	HashItem*               startSegm;		
	/* Number of partiotions. It should be power of 2 */
	uint                    partNum;
    hashFunc                hashFunc;	    /* hash function */
	hashCmpFunc             hashCmp;
	hashCpyFunc             hashCpy;		
	hashAllocFunc           hashAlloc;
	uint		            highMask;		
	uint		            lowMask;		
	uint                    numItemsToAlloc;
	/* Hashtable current state */
	ulong                   numItems;
    ulong                   numHashLists;
	/* This is a linked list of free elements.
	 * It can be newly allocated items or removed items 
	 * which were not freed but added into freeList to recycle */
	HashItem                freeList;
	Bool                    isWithoutExtention;
} SHashtable, *Hashtable;


#define HASH_FUNC	            0x001
#define HASH_CMP	            0x002
#define HASH_SEG	            0x004
#define HASH_KEYCPY	            0x008	
#define HASH_ALLOC		        0x010
#define HASH_LIST_SIZE          0x020
#define HASH_ITEM		        0x040
#define HASH_WITHOUT_EXTENTION  0x080
#define HASH_PARTITION          0x100
#define HASH_SHARED_MEMORY      0x200


#define GET_HASH_KEY(item)           ((char*)item + AlignDefault(sizeof(SHashItem)))
#define GET_HASH_VALUE(key, keyLen)  ((char*)key + AlignDefault(keyLen))
#define IS_TABLE_PARTITIONED(table)  ((table)->partNum != 0)


#define Max(x, y)		((x) > (y) ? (x) : (y))

#endif

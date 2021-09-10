
#include "hashtable.h"
#include "memnodes.h"
#include "spin.h"

typedef int slock_t;

#define NUM_FREELISTS			32

#define MOD(x,y) ((x) & ((y)-1))

/*
 * Key (also entry) part of a HASHELEMENT
 */
#define ELEMENTKEY(helem) (((char *)(helem)) + sizeof(HASHELEMENT))

/*
 * Hash functions must have this signature.
 */
typedef int (*HashValueFunc) (const void *key, int keysize);

/*
 * Key comparison functions must have this signature.  Comparison functions
 * return zero for match, nonzero for no match.  (The comparison function
 * definition is designed to allow memcmp() and strncmp() to be used directly
 * as key comparison functions.)
 */
typedef int (*HashCompareFunc) (const void *key1, const void *key2, int keysize);

/*
 * Key copying functions must have this signature.  The return value is not
 * used.  (The definition is set up to allow memcpy() and strlcpy() to be
 * used directly.)
 */
typedef void *(*HashCopyFunc) (void *dest, const void *src, int keysize);

/*
 * Space allocation function for a hashtable --- designed to match malloc().
 * Note: there is no free function API; can't destroy a hashtable unless you
 * use the default allocator.
 */
typedef void *(*HashAllocFunc) (int request);

/*
 * HASHELEMENT is the private part of a hashtable entry.  
 * The caller's data follows the HASHELEMENT structure.  
 * The hash key is expected to be at the start of the caller's hash entry data structure.
 */
typedef struct HASHELEMENT
{
	struct HASHELEMENT *link;	/* link to next entry in same bucket */
	int		hashvalue;		/* hash function result for this entry */
} HASHELEMENT;

/* A hash bucket is a linked list of HASHELEMENTs */
typedef HASHELEMENT *HASHBUCKET;

/* A hash segment is an array of bucket headers */
typedef HASHBUCKET *HASHSEGMENT;

/*
 * Per-freelist data.
 *
 * In a partitioned hash table, each freelist is associated with a specific
 * set of hashcodes, as determined by the FREELIST_IDX() macro below.
 * nentries tracks the number of live hashtable entries having those hashcodes
 * (NOT the number of entries in the freelist, as you might expect).
 *
 * The coverage of a freelist might be more or less than one partition, so it
 * needs its own lock rather than relying on caller locking.  Relying on that
 * wouldn't work even if the coverage was the same, because of the occasional
 * need to "borrow" entries from another freelist; see get_hash_entry().
 *
 * Using an array of FreeListData instead of separate arrays of mutexes,
 * nentries and freeLists helps to reduce sharing of cache lines between
 * different mutexes.
 */
typedef struct
{
	long		mutex;			/* spinlock for this freelist */
	long		nentries;		/* number of entries in associated buckets */
	HASHELEMENT *freeList;		/* chain of free elements */
} FreeListData;

/*
 * Header structure for a hash table --- contains all changeable info
 *
 * In a shared-memory hash table, the HASHHDR is in shared memory, while
 * each backend has a local HTAB struct.  For a non-shared table, there isn't
 * any functional difference between HASHHDR and HTAB, but we separate them
 * anyway to share code between shared and non-shared tables.
 */
struct HASHHDR
{
    /*
	 * The freelist can become a point of contention in high-concurrency hash
	 * tables, so we use an array of freelists, each with its own mutex and
	 * nentries count, instead of just a single one.  Although the freelists
	 * normally operate independently, we will scavenge entries from freelists
	 * other than a hashcode's default freelist when necessary.
	 *
	 * If the hash table is not partitioned, only freeList[0] is used and its
	 * spinlock is not used at all; callers' locking is assumed sufficient.
	 */
	FreeListData freeList[NUM_FREELISTS]; 

	/* These fields can change, but not in a partitioned table */
	/* Also, dsize can't change in a shared table, even if unpartitioned */
	long		dsize;			/* directory size */
	long		nsegs;			/* number of allocated segments (<= dsize) */
	int		    max_bucket;		/* ID of maximum bucket in use */
	int		    high_mask;		/* mask to modulo into entire table */
	int		    low_mask;		/* mask to modulo into lower half of table */

    /* These fields are fixed at hashtable creation */
	int		    keysize;		/* hash key length in bytes */
	int	  	    entrysize;		/* total user element size in bytes */
	long		num_partitions; /* # partitions (must be power of 2), or 0 */
	long		max_dsize;		/* 'dsize' limit if directory is fixed size */
	long		ssize;			/* segment size --- must be power of 2 */
	int			sshift;			/* segment shift = log2(ssize) */
	int			nelem_alloc;	/* number of entries to allocate at once */

	/*
	 * Count statistics here.  NB: stats code doesn't bother with mutex, so
	 * counts could be corrupted a bit in a partitioned table.
	 */
	long		accesses;
	long		collisions;
};

/* Parameter data structure for hash_create */
/* Only those fields indicated by hash_flags need be set */
typedef struct HASHCTL
{
	/* Used if HASH_PARTITION flag is set: */
	long		num_partitions; /* # partitions (must be power of 2) */
	/* Used if HASH_SEGMENT flag is set: */
	long		ssize;			/* segment size */
	/* Used if HASH_DIRSIZE flag is set: */
	long		dsize;			/* (initial) directory size */
	long		max_dsize;		/* limit to dsize if dir size is limited */
	/* Used if HASH_ELEM flag is set (which is now required): */
	int 		keysize;		/* hash key length in bytes */
	int		    entrysize;		/* total user element size in bytes */
	/* Used if HASH_FUNCTION flag is set: */
	HashValueFunc hash;			/* hash function */
	/* Used if HASH_COMPARE flag is set: */
	HashCompareFunc match;		/* key comparison function */
	/* Used if HASH_KEYCOPY flag is set: */
	HashCopyFunc keycopy;		/* key copying function */
	/* Used if HASH_ALLOC flag is set: */
	HashAllocFunc alloc;		/* memory allocator */
	/* Used if HASH_CONTEXT flag is set: */
	MemoryContext hcxt;			/* memory context to use for allocations */
	/* Used if HASH_SHARED_MEM flag is set: */
	struct HASHHDR    *hctl;			/* location of header in shared mem */
} HASHCTL;

#define IS_PARTITIONED(hctl)  ((hctl)->num_partitions != 0)

#define FREELIST_IDX(hctl, hashcode) \
	(IS_PARTITIONED(hctl) ? (hashcode) % NUM_FREELISTS : 0)

static HASHSEGMENT seg_alloc(struct HTAB *hashp);

/*
 * Top control structure for a hashtable --- in a shared table, each backend
 * has its own copy (OK since no fields change at runtime)
 */
struct HTAB
{
	struct HASHHDR    *hctl;		/* => shared control information */
	HASHSEGMENT       *dir;			/* directory of segment starts */
	HashValueFunc     hash;			/* hash function */
	HashCompareFunc   match;		/* key comparison function */
	HashCopyFunc      keycopy;		/* key copying function */
	HashAllocFunc     alloc;		/* memory allocator */
	MemoryContext     hcxt;			/* memory context if default allocator used */
	char	          *tabname;		/* table name (for error messages) */
	int	   	          isshared;		/* true if table is in shared memory */
	int		          isfixed;		/* if true, don't enlarge */

	/* freezing a shared table isn't allowed, so we can keep state here */
	int		          frozen;	    /* true = no more inserts allowed */

	/* We keep local copies of these fixed values to reduce contention */
	int		          keysize;		/* hash key length in bytes */
	long		      ssize;	    /* segment size --- must be power of 2 */
	int			      sshift;		/* segment shift = log2(ssize) */
};

/* hash_search operations */
typedef enum
{
	HASHACTION_FIND,
	HASHACTION_ENTER,
	HASHACTION_REMOVE,
	HASHACTION_ENTER_NULL
} HASHACTION;

static MemoryContext CurrentDynaHashCxt = NULL;

static void *DynaHashAlloc(int size)
{
	return malloc(size);
}

struct HTAB *hash_create(const char *tabname, long nelem, const HASHCTL *info, int flags)
{
    struct HTAB *hashp;
	struct HASHHDR *hctl;
    
	/* Initialize the hash header, plus a copy of the table name */
	hashp = (struct HTAB *)DynaHashAlloc(sizeof(struct HTAB) + strlen(tabname) + 1);
	memset(hashp, 0, sizeof(struct HTAB));
    
    
    return hashp;
}

int init_htab(struct HTAB *hashp, long nelem)
{
	int i;
	int nbuckets;
	int nsegs;
	HASHSEGMENT segp;
    struct HASHHDR *hctl = hashp->hctl;   

    if (IS_PARTITIONED(hctl))
		for (i = 0; i < NUM_FREELISTS; i++)
			SpinLockInit(&(hctl->freeList[i].mutex), 0);

	nbuckets = next_pow2_int(nelem);
	while (nbuckets < hctl->num_partitions)
		nbuckets <<= 1;
    
	hctl->max_bucket = hctl->low_mask = nbuckets - 1;
	hctl->high_mask = (nbuckets << 1) - 1;

    nsegs = (nbuckets - 1) / hctl->ssize + 1;
	nsegs = next_pow2_int(nsegs); 

    if (nsegs > hctl->dsize)
	{
        if (!(hashp->dir))
			hctl->dsize = nsegs;
		else
			return 0;
	}

	if (!(hashp->dir))
	{
        CurrentDynaHashCxt = hashp->hcxt;
		hashp->dir = (HASHSEGMENT *)hashp->alloc(hctl->dsize * sizeof(HASHSEGMENT));
		if (!hashp->dir)
			return 0;
	}
    
	for (segp = hashp->dir; hctl->nsegs < nsegs; hctl->nsegs++, segp++)
	{
        *segp = seg_alloc(hashp);
		if (*segp == NULL)
			return 0;
	}
    
	/* Choose number of entries to allocate at a time */
	hctl->nelem_alloc = choose_nelem_alloc(hctl->entrysize);

	return 1;
}

static int next_pow2_int(long num)
{
	if (num > INT_MAX / 2)
		num = INT_MAX / 2;
	return 1 << (log(num) / log(2));
}

static HASHSEGMENT seg_alloc(struct HTAB *hashp)
{
	HASHSEGMENT segp;

	CurrentDynaHashCxt = hashp->hcxt;
	segp = (HASHSEGMENT) hashp->alloc(sizeof(HASHBUCKET) * hashp->ssize);

	if (!segp)
		return NULL;

	memset(segp, 0, sizeof(HASHBUCKET) * hashp->ssize);

	return segp;
}

static int choose_nelem_alloc(int entrysize)
{
    int nelem_alloc;
	int elementSize;
	int allocSize;   

    elementSize = sizeof(HASHELEMENT) + entrysize;
    allocSize = 32 * 4;	

    do
	{
        allocSize <<= 1;
        nelem_alloc = allocSize / elementSize;
	} 
	while (nelem_alloc < 32);

    return nelem_alloc;
}

int calc_bucket(struct HASHHDR *hctl, int hash_val)
{
	int bucket;

	bucket = hash_val & hctl->high_mask;
	if (bucket > hctl->max_bucket)
		bucket = bucket & hctl->low_mask;

	return bucket;
}

HASHBUCKET get_hash_entry(struct HTAB *hashp, int freelist_idx)
{
    struct HASHHDR    *hctl = hashp->hctl;
	HASHBUCKET	newElement;     
    
    for (;;)
	{
        if (IS_PARTITIONED(hctl))
			SpinLockAcquire(&hctl->freeList[freelist_idx].mutex);
        
        newElement = hctl->freeList[freelist_idx].freeList;
        if (newElement != NULL)
			break;

		if (IS_PARTITIONED(hctl))
			SpinLockRelease(&hctl->freeList[freelist_idx].mutex);

		if (!element_alloc(hashp, hctl->nelem_alloc, freelist_idx))
		{
            int borrow_from_idx;

			if (!IS_PARTITIONED(hctl))
				return NULL;

            borrow_from_idx = freelist_idx;
			for (;;)
			{
                borrow_from_idx = (borrow_from_idx + 1) % NUM_FREELISTS;
				if (borrow_from_idx == freelist_idx)
					break;

                SpinLockAcquire(&(hctl->freeList[borrow_from_idx].mutex));
                newElement = hctl->freeList[borrow_from_idx].freeList;

				if (newElement != NULL)
				{
                    hctl->freeList[borrow_from_idx].freeList = newElement->link;
					SpinLockRelease(&(hctl->freeList[borrow_from_idx].mutex));

                    SpinLockAcquire(&hctl->freeList[freelist_idx].mutex);
					hctl->freeList[freelist_idx].nentries++;
					SpinLockRelease(&hctl->freeList[freelist_idx].mutex);

					return newElement;
				}

				SpinLockRelease(&(hctl->freeList[borrow_from_idx].mutex));
			}
		}

		return NULL;
	}

	return NULL;
}

void *hash_search_with_hash_value(struct HTAB *hashp, void *keyPtr, int hashvalue, HASHACTION action, int *foundPtr)
{
    int bucket;
	int keysize;
	long segment_num;
	long segment_ndx;
	HASHSEGMENT segp;
	HASHBUCKET currBucket;
	HASHBUCKET *prevBucketPtr;
	struct HASHHDR *hctl = hashp->hctl;
    HashCompareFunc match;

    int freelist_idx = FREELIST_IDX(hctl, hashvalue);

	if (action == HASHACTION_ENTER || action == HASHACTION_ENTER_NULL)
	{
        if (hctl->freeList[0].nentries > (long) hctl->max_bucket 
			&& !IS_PARTITIONED(hctl) 
			&& !hashp->frozen)
		{
			// (void) expand_table(hashp);
		}
	}
    
	bucket = calc_bucket(hctl, hashvalue);

	segment_num = bucket >> hashp->sshift;
	segment_ndx = MOD(bucket, hashp->ssize);
    segp = hashp->dir[segment_num];
    
	prevBucketPtr = &segp[segment_ndx];
	currBucket = *prevBucketPtr;
    keysize = hashp->keysize;

	while (currBucket != NULL)
	{
		if (currBucket->hashvalue == hashvalue &&
			match(ELEMENTKEY(currBucket), keyPtr, keysize) == 0)
			break;

		prevBucketPtr = &(currBucket->link);
		currBucket = *prevBucketPtr;
	}

	if (foundPtr)
	   *foundPtr = (int)(currBucket != NULL);

	switch (action)
	{
        case HASHACTION_FIND:
			if (currBucket != NULL)
				return (void *)ELEMENTKEY(currBucket);
			return NULL;

        case HASHACTION_REMOVE:
            if (currBucket != NULL)
			{
                if (IS_PARTITIONED(hctl))
					SpinLockAcquire(&(hctl->freeList[freelist_idx].mutex));
                
				hctl->freeList[freelist_idx].nentries--;

                *prevBucketPtr = currBucket->link;
                
                currBucket->link = hctl->freeList[freelist_idx].freeList;
				hctl->freeList[freelist_idx].freeList = currBucket;

                if (IS_PARTITIONED(hctl))
					SpinLockRelease(&hctl->freeList[freelist_idx].mutex);

                return (void *)ELEMENTKEY(currBucket);
			}
			return NULL;
        
        case HASHACTION_ENTER:
			if (currBucket != NULL)
				return (void *)ELEMENTKEY(currBucket);
            
			currBucket = get_hash_entry(hashp, freelist_idx);
			if (currBucket == NULL)
			{
                if (action == HASHACTION_ENTER_NULL)
					return NULL;
			}

            *prevBucketPtr = currBucket;
			currBucket->link = NULL;
			currBucket->hashvalue = hashvalue;

			hashp->keycopy(ELEMENTKEY(currBucket), keyPtr, keysize);
			return (void *)ELEMENTKEY(currBucket);
	}

    return NULL;
}

int element_alloc(struct HTAB *hashp, int nelem, int freelist_idx)
{
	struct HASHHDR    *hctl = hashp->hctl;
	int		elementSize;
	HASHELEMENT *firstElement;
	HASHELEMENT *tmpElement;
	HASHELEMENT *prevElement;
	int			i;

	if (hashp->isfixed)
		return 1;

	elementSize = sizeof(HASHELEMENT) + hctl->entrysize;

	CurrentDynaHashCxt = hashp->hcxt;
	firstElement = (HASHELEMENT *) hashp->alloc(nelem * elementSize);

	if (!firstElement)
		return 0;

	prevElement = NULL;
	tmpElement = firstElement;

	for (i = 0; i < nelem; i++)
	{
		tmpElement->link = prevElement;
		prevElement = tmpElement;
		tmpElement = (HASHELEMENT *) (((char *) tmpElement) + elementSize);
	}

	if (IS_PARTITIONED(hctl))
		SpinLockAcquire(&hctl->freeList[freelist_idx].mutex);

	firstElement->link = hctl->freeList[freelist_idx].freeList;
	hctl->freeList[freelist_idx].freeList = prevElement;

	if (IS_PARTITIONED(hctl))
		SpinLockRelease(&hctl->freeList[freelist_idx].mutex);

	return 1;
}
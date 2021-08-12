
#include "hashtable.h"
#include "memnodes.h"

typedef int slock_t;

#define NUM_FREELISTS			32

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

/*
 * Top control structure for a hashtable --- in a shared table, each backend
 * has its own copy (OK since no fields change at runtime)
 */
struct HTAB
{
	struct HASHHDR    *hctl;			/* => shared control information */
	int a;
};

struct HTAB *hash_create(const char *tabname, long nelem, const HASHCTL *info, int flags)
{
    struct HTAB h;
    return &h;
}

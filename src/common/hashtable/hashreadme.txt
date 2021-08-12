
dynahash.c 

 supports both local-to-a-backend hash tables and hash tables in shared memory.  
 For shared hash tables, it is the caller's responsibility
 to provide appropriate access interlocking.  
 The simplest convention is that a single LWLock protects the whole hash table.  
 Searches (HASH_FIND or ash_seq_search) need only shared lock, 
 but any update requires exclusive lock.  
 
 For heavily-used shared tables, the single-lock approach creates a concurrency bottleneck, 
 so we also support "partitioned" locking wherein
 there are multiple LWLocks guarding distinct subsets of the table.  
 
 To use a hash table in partitioned mode, the HASH_PARTITION flag must be given to hash_create.  
 This prevents any attempt to split buckets on-the-fly.
 
 Therefore, each hash bucket chain operates independently, 
 and no fieldscof the hash header change after init except nentries and freeList.
 
 (A partitioned table uses multiple copies of those fields, guarded by spinlocks, for additional concurrency.)
 
 This lets any subset of the hash buckets be treated as a separately lockable partition.  
 
 We expect callers to use the low-order bits of a lookup key's hash value as a partition number 
 
 --- this will work because of the way calc_bucket() maps hash values to bucket numbers.
 
 For hash tables in shared memory, the memory allocator function should
 match malloc's semantics of returning NULL on failure.  
 
 For hash tables in local memory, we typically use palloc() which will throw error on failure.
 The code in this file has to cope with both cases.
 
 dynahash.c provides support for these types of lookup keys:

 1. Null-terminated C strings (truncated if necessary to fit in keysize), 
    compared as though by strcmp().  
	This is selected by specifying the HASH_STRINGS flag to hash_create.
	
 2. Arbitrary binary data of size keysize, compared as though by memcmp().
    (Caller must ensure there are no undefined padding bits in the keys!)
    This is selected by specifying the HASH_BLOBS flag to hash_create.

 3. More complex key behavior can be selected by specifying user-supplied hashing, comparison, 
    and/or key-copying functions.  
	At least a hashing function must be supplied; 
	comparison defaults to memcmp() and key copying to memcpy() 
	when a user-defined hashing function is selected.

Compared to simplehash, dynahash has the following benefits:
 
 - It supports partitioning, which is useful for shared memory access using locks.
 - Shared memory hashes are allocated in a fixed size area at startup 
   and are discoverable by name from other processes.
 - Because entries don't need to be moved in the case of hash conflicts,
   dynahash has better performance for large entries.
 - Guarantees stable pointers to entries.
  	
Original comments:

Dynamic hashing, after CACM April 1988 pp 446-457, by Per-Ake Larson.
Coded into C, with minor code improvements, and with hsearch(3) interface,
by ejp@ausmelb.oz, Jul 26, 1988: 13:16;
also, hcreate/hdestroy routines added to simulate hsearch(3).

These routines simulate hsearch(3) and family, with the important
difference that the hash table is dynamic - can grow indefinitely
beyond its original size (as supplied to hcreate()).

Performance appears to be comparable to that of hsearch(3).
The 'source-code' options referred to in hsearch(3)'s 'man' page
are not implemented; otherwise functionality is identical.

Compilation controls:

 HASH_DEBUG controls some informative traces, mainly for debugging.
 HASH_STATISTICS causes HashAccesses and HashCollisions to be maintained;
 when combined with HASH_DEBUG, these are displayed by hdestroy().

 Problems & fixes to ejp@ausmelb.oz. 
 
 WARNING: 
    relies on pre-processor concatenation property, 
	in probably unnecessary code 'optimization'.
	
	

 
 
 
 
 
 
 
 
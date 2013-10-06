
#include "hashtablemanager.h"

const SIHashtableManager sHashtableManager = 
{ 
	&sTrackMemManager,
	&sCommonHelper,
	&sHashtableHelper,
	createHashtable,
    hashFind,
    hashInsert
};

const IHashtableManager hashtableManager = &sHashtableManager;

void setDefaults(Hashtable tbl)
{
    tbl->hashFunc = hashFuncStr;
    tbl->hashCmp = memcmp;
    tbl->hashCpy = memcpy;

    tbl->isInShared = False;
	tbl->noInserts = True;

	tbl->keyLen = sizeof(char *);
	tbl->valLen = 2 * tbl->keyLen;

    tbl->segmSize = DEFAULT_SEG_SIZE;
    tbl->segmShift = DEFAULT_SEG_SHIFT;

	tbl->hashListSize = DEFAULT_HASH_LIST_SIZE;
    tbl->segmsAmount = DEFAULT_SEGS_AMOUNT;
	tbl->maxSegmsAmount = NO_SEGS_AMOUNT_RESTRICTIONS;
	tbl->isWithoutExtention = True;
}

Hashtable createHashtable(
    void*              self,
	char*              name, 
	long               maxItemsNum, 
	HashtableSettings  set, 
	int                setFlags)
{
	IHashtableManager _ = (IHashtableManager)self;
	Hashtable         tbl;
	int               nHashLists;
	int               nSegs;
	int               segLLSize;
	uint              elemSize;
    HashSegment*      segP; 
	ulong             tblSegSize;

	/* Initialize the hash header, plus a copy of the table name.
	 * First sizeof(Hashtable) bytes are allocated for a hash table.
	 * The next strlen(Name) are allocated for the name.
	 * In this way we unate one malloc call for struct SHashtable with another 
	 * malloc for char* name.
	 */
	tbl = (Hashtable)_->memManager->alloc(sizeof(SHashtable) + strlen(name) + 1);
	memset(tbl, 0, sizeof(SHashtable));

	/* Adding one to a pointer means "produce a pointer to the object 
	 * that comes in memory right after this one," 
	 * which means that the compiler automatically scales up 
	 * whatever you're incrementing the pointer with 
	 * by the size of the object being pointed at. */
	tbl->name = (char*)(tbl + 1);
	strcpy(tbl->name, name);

    setDefaults(tbl);

	if (setFlags & HASH_FUNC)
		tbl->hashFunc = set->hashFunc;

	if (tbl->hashFunc == hashFuncStr)
		tbl->hashCmp = hashCmpFuncStr;

	if (setFlags & HASH_CMP)
		tbl->hashCmp = set->hashCmp;

    if (tbl->hashFunc == hashFuncStr)
		tbl->hashCpy = (hashCpyFunc)strcpy;

    if (setFlags & HASH_KEYCPY)
		tbl->hashCpy = set->hashCpy;

	if (setFlags & HASH_ITEM)
	{
	    tbl->keyLen = set->keyLen;
	    tbl->valLen = set->valLen;
	}

    if (setFlags & HASH_SEG)
	{
		tbl->segmSize = set->segmSize;
		tbl->segmShift = set->segmShift;
	}

    if (setFlags & HASH_LIST_SIZE)
	{
		tbl->segmsAmount = set->segmsAmount;
		tbl->maxSegmsAmount = set->maxSegmsAmount;
		tbl->hashListSize = set->hashListSize;
	}

	if (setFlags & HASH_WITHOUT_EXTENTION)
	{
        tbl->isWithoutExtention = set->isWithoutExtention;
	}

	nHashLists = (maxItemsNum - 1) / tbl->hashListSize + 1;
    nHashLists = _->commonHelper->nextPowerOf2(nHashLists);

	tbl->lowMask  = _->hashtableHelper->calcLowMask(nHashLists);
	tbl->highMask = _->hashtableHelper->calcHighMask(nHashLists);

	nSegs = _->hashtableHelper->calcSegmsNum(nHashLists, tbl->segmSize);

	tbl->numHashLists = nHashLists;
	tbl->segments = (AHashSegment)_->memManager->alloc(tbl->segmsAmount * sizeof(HashSegment));
    
    tblSegSize = tbl->segmSize;
	/* Allocate initial segments */
	for (segP = tbl->segments; tbl->nSegs < nSegs; tbl->nSegs++, segP++)
	{
		segLLSize = tblSegSize * sizeof(HashList);
		*segP = (HashSegment)(AHashList)_->memManager->alloc(segLLSize);
	    memset(*segP, 0, segLLSize);
	}
    
    /* Our element consists of a header and data sp that the size 
     * is sum of the header's size and data's size. 
	 * Header also includes aligned key. */
    elemSize = ALIGN(sizeof(SHashItem)) + ALIGN(tbl->keyLen) + ALIGN(tbl->valLen);
	tbl->numItemsToAlloc = itemsNumToAlloc(elemSize);

	return tbl;
}

/* Convert a hash value to a hash list number */
uint convertHashToListNumber(
	Hashtable     tbl, 
	uint          hash)
{
	uint		  hashListNum;
    
	/* 1. From the start when the table expansion has not occured yet.
	 *    In this case we have:
	 *       lowMask = numLists - 1, highMask = 2*numLists - 1
	 *    when we bitwising hash with the highmask we will receive a number 
	 *    from [0...highMask] with equal probability. If we exceed numLists,
	 *    we mod it by lowMask. 
	 *    |..............|...............|
	 *    0            numLs           2numLs
	 *    We need to map a hash uniformly to [0...numLs]
	 *    The easiest way is  hash mod (numLs - 1)
	 *    The following steps are also correct: 
	 *    hash mod (2numLs - 1) produces a number from [0...2numLs-1]
	 *    Let's define this number as x. If x > numLs - 1 then  
	 *    (x - (numLs - 1)) or x mod (numLs - 1)
	 *
	 * 2. General case. When we have extended the table several times, 
	 *    hashListNum is greater than (numLs - 1) and hashListNum < 2numLs - 1
	 *    For example:
	 *    |..............|....|..........|
	 *   /              /    /          / 
	 *  0       originNum  numLists    2originNum
	 *    Our goal is to map a hash value to interval [0...numLists]
	 *    This is the same as hash mod (numLists - 1). But 
	 *    we need to evaluate this operation only by bitwise operations.
	 *    We do hash mod highMask. If we get a number from [0...numLists],
	 *    we stop our process. The probability is numLists / 2originNum.
	 *    
	 */
	hashListNum = hash & tbl->highMask; 
	if (hashListNum == 16)
	{
        hashListNum = hashListNum;
	}
	if (hashListNum > tbl->numHashLists - 1)
		hashListNum = hashListNum & tbl->lowMask;

	return hashListNum;
}

/* This function computes a number of items to allocate
 * when we need to extend a hashtable. For perfomance care 
 * we need to allocate a large enough piece of memory 
 * to avoid frequent malloc calls. 
 * We need to alloc at least 32 items. 
 * Suppose we are going to allocate (32 + k) hash items, where k >= 0
 * A total size of memory that will be allocated should be a power of two.
 */
uint itemsNumToAlloc(uint elemSize)
{  
    uint elemsToAlloc;
    uint allocMemSize;

    /* Suppose that our elemSize is at least 8 bits.
	 * Then supppose we allocate 32 items.
	 * In this case a memory to allocate is 32 * 8. 
	 * Now we need to check if our assumption was correct:
	 * Divide (32 * 8) on elemSize. If a number is more than 32
	 * that means that our elemSize is less than 8 and we return it.
	 * If the number is less than 32 we multiple allocMemSize for 2 
	 * and check again until we receive a number more or equal 32
	 */

	allocMemSize = 32 * 4;

    do
    {
	   allocMemSize <<= 1;
	   elemsToAlloc = allocMemSize / elemSize;
    } 
	while (elemsToAlloc < 32);

    return elemsToAlloc;
}

void allocNewItems(
    void*             self,
    Hashtable         tbl)
{
    IHashtableManager  _ = (IHashtableManager)self;
    HashItem           prevItem, currItem, firstItem, newElem;
	int                i = 0;
	uint               nelems = tbl->numItemsToAlloc;

	/* A hash element memory looks like:
	 * [... aligned header ...][... aligned key ...][... aligned data ...] */
	uint elSize = ALIGN(sizeof(SHashItem)) + ALIGN(tbl->keyLen) + ALIGN(tbl->valLen);
	firstItem = (HashItem)_->memManager->alloc(nelems * elSize);

	/* We allocated memory for 'nelems' number of items.
	 * Here we need to form a linked list from this memory:
	 * Divide this memory into 'nelems' parts 
	 * and set the HashItem's link to a neighbour part.
	 * Algorithm is very simple:
	 *   Numerate all parts from 0 to nelems - 1 
	 *   and set inward links as follows:   
	 *   NULL <- item0 <- item1 <- ... <- itemn-2 <- itemn-1
	 */
	prevItem = NULL;
	currItem = firstItem;
	while (i++ < nelems)
	{
	   currItem->next = prevItem;
	   prevItem = currItem;
	   /* Move to the next part of memory */
	   currItem = (HashItem)((char*)currItem + elSize);
	}

	/* It is possible that freeList has been just created 
	 * by another thread simultaneously and it is not empty
	 * The linked list of new memory:
	 * NULL <- item0 <- item1 <- ... <- itemn-2 <- itemn-1
	 *           \                                   \
	 *          firstItem                           prevItem
	 * freeList is NULL or some not empty linked list
	 *         free0 -> free1 -> ..... -> freek 
	 * freeList /  
	 * First of all we add freeList to the firstItem and then
	 * set freeList pointer to prevItem
	 */
	firstItem->next = tbl->freeList;
	tbl->freeList = prevItem;
}

HashItem* hashFindInternal(
	Hashtable         tbl, 
    void*             key)
{
	HashSegment       segm;
	HashItem*         itemPtr;
	HashItem          item;
    uint              keyLen  = tbl->keyLen;
	hashCmpFunc       cmpFunc = tbl->hashCmp;
    uint              hash    = tbl->hashFunc(key, keyLen);
    uint              listId, sNum, sInd;

	listId = convertHashToListNumber(tbl, hash);

	/* Segment size always is a power of 2: 2^n.
	 * SegmentSize  is 2^n.
	 * SegmentShift is n.
	 * listId we can represent: listId = segmSize * sNum + sInd;
	 * So sNum = listId/segmSize, sInd = listId mod segmSize;
	 * When we operate with powers of 2, we can compute these operations
	 * using only bitwise operations
	 */
	sNum = listId >> tbl->segmShift; 
	sInd = listId & (tbl->segmSize - 1);

	segm = tbl->segments[sNum];
    itemPtr = &segm[sInd];
    item = *itemPtr;

	while (item != NULL)
	{
		if (item->hash == hash && cmpFunc(GET_HASH_KEY(item), key, keyLen) == 0)
			break;

		itemPtr = &(item->next);
		item = *itemPtr;
	}

	return itemPtr;
}

void* hashFind(
	Hashtable         tbl, 
    void*             key)
{
	HashItem* itemPtr = hashFindInternal(tbl, key);
    HashItem  item = *itemPtr;

	if (item != NULL)
       return GET_HASH_KEY(item);

    return NULL;
}

void extendHashTable(
    void*          self,
    Hashtable      tbl)
{
    IHashtableManager  _              = (IHashtableManager)self;
	uint               newHashListNum = tbl->numHashLists + 1;
	uint               oldHashListNum = 0;
	/* newSNum is expected segment number in case we add new list */
	uint               newSNum        = newHashListNum >> tbl->segmShift; 
	uint	           newSInd        = newHashListNum & (tbl->segmSize - 1);
	uint               oldSNum, oldSInd;
    int                segLLSize;
    HashSegment        oldSegm, newSegm; 
    HashItem          *oldListPtr, *newListPtr;
    HashItem           currElem, nextElem;

	/* if expected segment number is larger 
	 * than current amount of segments, we need to allocate 
	 * a new segment. tbl->nSegs is a number of segments with allocated memory. */
	if (newSNum >= tbl->nSegs)
	{
        /* tbl->segmsAmount is the number of items in the segments array.
		 * If new segment number exceeds the length of our segments array,
		 * we need to reallocate this array. */
	    if (newSNum >= tbl->segmsAmount)
			;
        
		segLLSize = tbl->segmSize * sizeof(HashList);
		tbl->segments[newSNum] = (HashSegment)(AHashList)_->memManager->alloc(segLLSize);
        memset(tbl->segments[newSNum], 0, segLLSize); 
		tbl->nSegs++;
	}

	tbl->numHashLists++;
    
	/* lowMask is a power of 2 minus 1, so it looks like:
	 * 0000111...111 n times. lowMask = hashListItemsNum - 1
	 * newHashListsNum is larger than hashListItemsNum.
	 * We define oldHashListNum in this way: 
	 * newHashListNum mod hashListItemsNum
	 * Taking into account that hashListItemsNum is a power of 2,
	 * we can compute mod by bitwise &ing.   
	 *                            newNumLists
	 *                           /                   
	 *    |..............|....|.|........|
	 *   /              /    /          / 
	 *  0       originNum  numLists    2originNum
	 */
	oldHashListNum = newHashListNum & tbl->lowMask;

    /* If we continue inserting new hashLists, our oldHashListNum increases
	 * and it can reach its maximum newHashListNum - 1 and after that wraps around.
	 * To prevent this we need to rearrange masks. 
	 * Suppose that tbl->highMask = 111...111, length is n.
	 * When newHashListNum is more than tbl->highMask, 
	 * it is 111...1111 + 1 = 1000...000, where we have n 0's 
	 * By bitwise or-ing them we will get 111...111 with length of n elements. */
	if (newHashListNum > tbl->highMask)    
	{
		tbl->lowMask = tbl->highMask;
		tbl->highMask = newHashListNum | tbl->lowMask;
	}

    /*
	 * Relocate records to the new bucket.	NOTE: because of the way the hash
	 * masking is done in calc_bucket, only one old bucket can need to be
	 * split at this point.  With a different way of reducing the hash value,
	 * that might not be true!
	 */
	oldSNum = oldHashListNum >> tbl->segmShift;
	oldSInd = oldHashListNum & (tbl->segmSize - 1);

	oldSegm = tbl->segments[oldSNum];
	newSegm = tbl->segments[newSNum];

	oldListPtr = &oldSegm[oldSInd];
	newListPtr = &newSegm[newSInd];

	for (currElem = *oldListPtr;
		 currElem != NULL;
		 currElem = nextElem)
	{
		nextElem = currElem->next;
		if (convertHashToListNumber(tbl, currElem->hash) == oldHashListNum)
		{
			*oldListPtr = currElem;
			oldListPtr = &currElem->next;
		}
		else
		{
			*newListPtr = currElem;
			newListPtr = &currElem->next;
		}
	}

	*oldListPtr = NULL;
    *newListPtr = NULL;
}

void* hashInsert(
	void*             self,
    Hashtable         tbl, 
    void*             key)
{
    HashItem*    itemPtr;
    HashItem     item;
    HashItem     newElem = tbl->freeList;
	uint         keyLen  = tbl->keyLen;
    uint         hash    = tbl->hashFunc(key, keyLen);
    
	/* We assume that our hash function produces completely 
	 * uniformly distributed data. We can compute the average
	 * amount of items in each hash list.
	 * tbl->hashListSize is expected average hash list size.
	 * If we exceed this number, we need to add a new hash list */
	if (tbl->numItems > tbl->hashListSize * tbl->numHashLists && !tbl->isWithoutExtention)
	   extendHashTable(self, tbl);

	itemPtr = hashFindInternal(tbl, key);
    item    = *itemPtr;

	if (item != NULL)
       return GET_HASH_KEY(item);

	if (newElem == NULL)
	   allocNewItems(self, tbl);

    newElem = tbl->freeList;      

	/* Remove the first entry from freeList */
	tbl->freeList = newElem->next;
	tbl->numItems++;

	*itemPtr = newElem;

	newElem->next = NULL;
	newElem->hash = hash;
            
    /* SHashItem is a struct with wo fileds: next and hash.
     * There are no other fields. We want to attach the whole key
     * to the end of the memory the pointer to SHashItem points to.
     * Before we need to align already allocated memory.
     * Memory will look like here:
     * 
     * [...next...][...hash....][..aligned..][....aligned..key.......][..aligned..value....] 
     * |<------ SHashItem ---->|            |
     * |<--------- aligned SHashItem ------>|
     */
	tbl->hashCpy(GET_HASH_KEY(newElem), key, keyLen);
	return GET_HASH_KEY(newElem);
}


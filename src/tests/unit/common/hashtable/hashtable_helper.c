#include "unity_fixture.h"
#include "hashtable.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"
#include "hashtable_helper.h"

SListItemCount hashListItemsCount[1000];
int            listsCount = 0;

void clearHashLists()
{
	int i;
	for (i = 0; i < listsCount; i++)
	{
		memset(&hashListItemsCount[i], 0, sizeof(SListItemCount));
	}
}

int calculateListCount(HashItem listPtr)
{
	int count = 0;
    HashItem currPtr = listPtr;
	while (currPtr != NULL)
	{
        count++;
		currPtr = currPtr->next;
	}
	return count;
}

void checkListCountDeviation(ListItemCount item)
{
    TEST_ASSERT_FALSE(item->deviation > MAX_ALLOWED_DEVIATION);
}

void calculateHashListsLens(Hashtable tbl, assertListCount assertFunc)
{
	int            i, j;
	int            nSegs       = tbl->nSegs;
	int            maxListNum  = tbl->lowMask;
	int            segSize     = tbl->segmSize;
    HashSegment    segm;
    HashItem       listPtr; 
	int            listId;
	int            totalCount = 0;
    ListItemCount  lCount;
	int            averageCount = tbl->numItems / (maxListNum + 1);

	if (tbl == NULL)
		return;

	for (i = 0; i < nSegs; i++)
	{
		segm = tbl->segments[i];
        for (j = 0; j < segSize; j++)
	    {
			listId  = segSize * i + j;
			if (listId > tbl->numHashLists - 1)
                break;

            listPtr = segm[j];
            lCount  = &hashListItemsCount[listId];
			lCount->listId = listId;
			lCount->count = calculateListCount(listPtr);
			lCount->deviation = ((float)abs(lCount->count - averageCount)) / ((float)abs(averageCount));

			if (assertFunc != NULL)
                assertFunc(lCount);
	    }
	}

    listsCount = listId + 1;
}


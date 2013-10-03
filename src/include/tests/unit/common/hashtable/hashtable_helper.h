
#ifndef HASHTABLE_HELPER_H
#define HASHTABLE_HELPER_H

#define MAX_ALLOWED_DEVIATION (0.1)

typedef struct SListItemCount
{
	uint       listId;	    
	uint	   count;
	float      deviation;
} SListItemCount, *ListItemCount;

typedef void (*assertListCount)(ListItemCount item);

extern SListItemCount hashListItemsCount[1000];
extern int            listsCount;

void clearHashLists();
void calculateHashListsLens(Hashtable tbl, assertListCount assertFunc);
void checkListCountDeviation(ListItemCount item);
void fillTableToExpansion(Hashtable tbl, IHashtableManager man, int* currentKey);

#endif
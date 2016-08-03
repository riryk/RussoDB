#include "listmanager.h"
#include "nodes.h"
#include "errorlogger.h"
#include "trackmemmanager.h"

const SIListManager sListManager = 
{ 
	&sErrorLogger,
    &sTrackMemManager,
    getListHead,
	listAppend
};

const IListManager listManager = &sListManager;

ListCell getListHead(List list)
{
	if (list != NULL)
		return list->head;

	return NULL;
}

List createList(void* self, ENodeType type)
{ 
    IListManager   _      = (IListManager)self;
    IMemoryManager memMan = (IMemoryManager)_->memManager;

	List	    newList;
	ListCell    newHead;

	newHead = (ListCell)memMan->alloc(sizeof(*newHead));
    newHead->next = NULL;

	newList = (List)memMan->alloc(sizeof(*newList));
	newList->type   = type;
	newList->length = 1;
	newList->head   = newHead;
	newList->tail   = newHead;

	return newList;
}

void addCellToTail(void* self, List list)
{
    IListManager   _      = (IListManager)self;
    IMemoryManager memMan = (IMemoryManager)_->memManager;

	ListCell   newItem;

	newItem       = (ListCell)memMan->alloc(sizeof(newItem));
	newItem->next = NULL;

    list->tail->next = newItem;
	list->tail       = newItem;
	list->length++;
}

List listAppend(void* self, List list, void* data)
{
	IListManager _     = (IListManager)self;
    IErrorLogger erlog = _->errorLogger;

	Bool isList = IsPointerList(list);
    erlog->assert(isList);

	if (list == (List)NULL)
        list = createList(_, T_List);
	else
        addCellToTail(_, list);

	list_first(list->tail) = data;
	return list;
}
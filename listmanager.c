#include "listmanager.h"

ListCell getListHead(List list)
{
	if (list != NULL)
		return list->head;

	return NULL;
}

List createList(void* self, ENodeType type)
{ 
    IListManager   _      = (IListManager)self;
    IMemoryManager memMan = (IMemoryManager)self;

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

List listAppend(void* self, List list, void* data)
{
	IListManager _     = (IListManager)self;
    IErrorLogger erlog = _->errorLogger;

	Bool isList = IsPointerList(list);
    erlog->assertCond(isList);

	if (list == (List)NULL)
		list = new_list(T_List);
	else
		new_tail_cell(list);

	lfirst(list->tail) = datum;
	check_list_invariants(list);
	return list;
}
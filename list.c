#include "list.h"

ListCell getListHead(List list)
{
	if (list != NULL)
		return list->head;

	return NULL;
}

/*
 * Append a pointer to the list. A pointer to the modified list is
 * returned. Note that this function may or may not destructively
 * modify the list; callers should always use this function's return
 * value, rather than continuing to use the pointer passed as the
 * first argument.
 */
void appendToList(List list, void* data)
{
	Assert(IsPointerList(list));

	if (list == NIL)
		list = new_list(T_List);
	else
		new_tail_cell(list);

	lfirst(list->tail) = datum;
	check_list_invariants(list);
	return list;
}

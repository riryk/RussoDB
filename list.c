#include "list.h"

ListCell getListHead(List list)
{
	if (list != NULL)
		return list->head;

	return NULL;
}


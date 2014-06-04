
#ifndef LISTMANAGER_H
#define LISTMANAGER_H

#include "list.h"
#include "ilistmanager.h"

extern const SIListManager sListManager;
extern const IListManager  listManager;

ListCell getListHead(List list);
List listAppend(void* self, List list, void* data);

#endif
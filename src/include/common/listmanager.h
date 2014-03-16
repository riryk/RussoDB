
#ifndef LISTMANAGER_H
#define LISTMANAGER_H

#include "list.h"

ListCell getListHead(List list);
void listAppend(void* self, List list, void* data);

#endif
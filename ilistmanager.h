
#ifndef ILISTMANAGER_H
#define ILISTMANAGER_H

#include "list.h"
#include "ierrorlogger.h"
#include "imemorymanager.h"

typedef struct SIListManager
{
	IErrorLogger   errorLogger;
    IMemoryManager memManager;

	ListCell (*getListHead)(List list);
    void (*listAppend)(void* self, List list, void* data);
} SIListManager, *IListManager;

#endif
#include <stdlib.h>
#include <stdio.h>
#include "memutils.h"

void MemoryContextCreate(MemoryContext node,
					const MemoryContextMethods *methods,
					MemoryContext parent,
					const char *name)
{
	node->isReset = 1;
	node->methods = methods;
	node->parent = parent;
	node->firstchild = NULL;
	node->mem_allocated = 0;
	node->prevchild = NULL;
	node->name = name;
	node->ident = NULL;

	if (parent)
	{
		node->nextchild = parent->firstchild;
		if (parent->firstchild != NULL)
			parent->firstchild->prevchild = node;
		parent->firstchild = node;
		node->allowInCritSection = parent->allowInCritSection;
	}
	else
	{
		node->nextchild = NULL;
		node->allowInCritSection = 0;
	}
}
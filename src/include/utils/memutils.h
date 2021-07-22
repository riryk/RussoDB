#include "memnodes.h"

extern void MemoryContextCreate(MemoryContext node,
								const MemoryContextMethods *methods,
								MemoryContext parent,
								const char *name);


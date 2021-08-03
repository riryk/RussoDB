#include "memnodes.h"

extern void MemoryContextCreate(MemoryContext node,
								const MemoryContextMethods *methods,
								MemoryContext parent,
								const char *name);

extern MemoryContext AllocSetContextCreateInternal(MemoryContext parent,
												   const char *name,
												   int minContextSize,
												   int initBlockSize,
												   int maxBlockSize);

#define AllocSetContextCreate \
	AllocSetContextCreateInternal






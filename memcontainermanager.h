#include "imemcontainermanager.h"

extern unsigned char Log2Table[256];

#define ASSERT(logger, condition, retval) \
	if (!(condition)) \
    { \
	   (logger)->assert((condition)); \
	   return (retval); \
    }

#define ASSERT_ARG(logger, condition, retval) \
	if (!(condition)) \
    { \
	   (logger)->assertArg((condition)); \
	   return (retval); \
    }

MemoryContainer memContCreate(
	void*                self,
    MemoryContainer      container,
	MemoryContainer      parent,
    MemContType          type, 
	size_t               size,
	char*                name,
	void*                (*malloc)(size_t size));

void* allocateMemory(
    void*                self,
    MemoryContainer      container, 
	size_t               size);

void resetMemoryFromSet(MemorySet set);

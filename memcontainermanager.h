#include "imemcontainermanager.h"

extern unsigned char Log2Table[256];

extern MemoryContainer topMemCont;
extern MemoryContainer currentMemCont;

#define ASSERT(logger, condition, retval) \
	if (!(condition)) \
    { \
	   (logger)->assert((condition)); \
	   return (retval); \
    }

#define ASSERT_VOID(logger, condition) \
	if (!(condition)) \
    { \
	   (logger)->assert((condition)); \
	   return; \
    }

#define ASSERT_ARG_VOID(logger, condition) \
	if (!(condition)) \
    { \
	   (logger)->assertArg((condition)); \
	   return; \
    }

#define ASSERT_ARG(logger, condition, retval) \
	if (!(condition)) \
    { \
	   (logger)->assertArg((condition)); \
	   return (retval); \
    }

void ctorMemContMan(
    void*            self, 
	FMalloc          funcMallocParam,
	FFree            funcFreeParam);

void resetErrCont(void* self);

MemoryContainer changeToErrorContainer();

void dtorMemContMan(
    void*            self);

MemoryContainer memContCreate(
	void*                self,
    MemoryContainer      container,
	MemoryContainer      parent,
    MemContType          type, 
	size_t               size,
	char*                name);

MemorySet memSetCreate(
    void*                self,
    MemoryContainer      container,
    MemoryContainer      parent,
    char*                name,
	size_t               minContainerSize,
	size_t               initBlockSize,
	size_t               maxBlockSize);

void* allocateMemory(
    void*                self,
    MemoryContainer      container, 
	size_t               size);

void showMemStat(
	void*                self,
    MemoryContainer      container,
	int                  level);

void printSetStatistic(
    MemorySet            set, 
	int                  level);

void resetMemoryFromSet(
	void*                self,
	MemorySet            set);

void freeChunk(
    void*                self,
	void*                mem);


#include "isharedmemmanager.h"

SharMemHeader SharMemCreate(
	void*           self,
	size_t          size, 
	Bool            makePrivate, 
	int             port);

void initSharMemAccess(void* sharMem);

void* allocSharedMem(
	void*        self,
	size_t       size);

size_t sizeMultiply(
	void*        self,
	size_t       s1, 
	size_t       s2);

size_t addSize(
    void*        self,
    size_t       s1, 
	size_t       s2);
#include "isharedmemmanager.h"

SharMemHeader SharMemCreate(
	void*           self,
	size_t          size, 
	Bool            makePrivate, 
	int             port);

void initSharMemAccess(void* sharMem);
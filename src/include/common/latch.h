
#include <stdlib.h>
#include <string.h>
#include "common.h"

#ifdef _WIN32
#include <windows.h>
#endif


#ifndef LATCH_H
#define LATCH_H

typedef struct SLatch
{
	int         is_set;
	Bool		is_shared;
	int			owner_pid;

#ifdef _WIN32
    HANDLE		event;
#endif

} SLatch, *Latch;

#endif


#include <stdlib.h>
#include <string.h>
#include "common.h"

#ifdef _WIN32
#include <windows.h>
#endif


#ifndef LATCH_H
#define LATCH_H

/* This structure is used to notify other threads or processes
 * Windows has inward mechanizm for notifying other processes or threads.
 * A structure which wraps window events. 
 */
typedef struct SLatch
{
	int         isset;
	Bool		shared;
	int			ownerid;

#ifdef _WIN32
    HANDLE		event;
#endif

} SLatch, *Latch;

#endif

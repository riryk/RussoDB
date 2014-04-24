
#include "ilightlockmanager.h"
#include "spinlockmanager.h"

#ifndef LIGHTLOCKMANAGER_H
#define LIGHTLOCKMANAGER_H

typedef enum ELightLockType
{
    BufFreelistLock,
    SharMemIndexLock,
    OidGenLock,
    XidGenLock
} ELightLockType;

typedef enum ELightLockMode
{
	LL_Exclusive,
	LL_Shared,

    /* a special lock mode which is used 
	 * when waiting for lock to become free.
	 */ 
	LL_WaitUntilFree,
	LL_LightLocksFixedNum
} ELightLockMode;

typedef struct SLightLock
{
	SpinLockType   mutex;			   /* Protects LWLock and queue of PGPROCs */
	Bool		   releaseOK;		   /* T if ok to release waiters */
	char		   exclHoldersNum;	   /* Number of exclusive holders (0 or 1) */
	int			   sharedHoldersNum;   /* Number of shared holders (0..MaxBackends) */
	ProcBackData   head;			   /* head of the list of waiting processes */
	ProcBackData   tail;			   /* tail of the list of waiting processes */
} SLightLock, *LightLock;

/* We keep all light locks in an array in shared memory.
 * We wont to keep it as power of 2. It ensures that individual 
 * cache items do not cross the cache lines boundaries.
 * On all platforms Light Weight lock size is between 16 and 32.
 */
#define LIGHT_LOCK_PADDED_SIZE (sizeof(SLightLock) <= 16 ? 16 : 32)

/* maximum number of locks that can be taken simultaneously. */
#define MAX_SIMULTANEOUS_LOCKS	100

/* This union is created to round up the light lock size
 * to the power of 2 (16 or 32).
 */
typedef union ULightLockPadded
{
	SLightLock	lock;
	char		pad[LIGHT_LOCK_PADDED_SIZE];
} ULightLockPadded;

#endif
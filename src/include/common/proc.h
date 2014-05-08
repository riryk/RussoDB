#include "common.h"
#include "semaphore.h"

#ifndef PROC_H
#define PROC_H

#define MAX_SOCKETS	64
#define NUM_AUXILIARY_PROCS 4

typedef struct SDeadChildInfo
{
	HANDLE		  waitHandle;
	HANDLE		  procHandle;
	DWORD		  procId;
} SDeadChildInfo, *DeadChildInfo;

typedef struct SBackendParams
{
	char		  dataDir[MAX_PATH];
	socket_type	  listenSockets[MAX_SOCKETS];
	ulong		  cancelKey;
	int			  childSlot;

#ifdef _WIN32
	HANDLE        segmId;
#endif

	void*         segmAddr;
	int		      processId;
	double        startTime;
	double        reloadTime;
    int64	      loggerFileTime;
	Bool		  redirectDone;
	Bool		  IsBinaryUpgrade;
	int			  maxSafeFileDescriptors;

#ifdef _WIN32
	HANDLE		  masterHandle;
	HANDLE		  initPipe;
	HANDLE        logPipe[2];
#endif

	char		  execPath[MAX_PATH];
	char		  libraryPath[MAX_PATH];
	char		  otions[MAX_PATH];
} SBackendParams, *BackendParams;

typedef struct SProcBackData
{  
	/* A semaphore to sleep on */
	TSemaphore  sem;	
	/* backend's process ID */
    int			procId;			 
	int			procNo; 
    int         backendId;
	uint        databaseId;
	uint        roleId;
    /* true if the process is  waiting for an LW lock */
	Bool		waitingForLightLock; 
	/* the light lock mode being waited for */
	uint8		waitingMode;
    /* next waiter for same light weight lock */
	struct SProcBackData* lightLockNext;
} SProcBackData, *ProcBackData;

#endif
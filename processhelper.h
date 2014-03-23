#include "common.h"
#include <stdlib.h>
#include "iprocesshelper.h"

#define MAX_SOCKETS	64

typedef struct SRelFileInfo
{
	int		      tblSpaceId;
	int 	      databaseId;
	int		      relId;		
} SRelFileInfo, *RelFileInfo;

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

int startSubProcess(void* self, int argc, char* argv[]);
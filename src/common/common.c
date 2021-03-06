
#include "common.h"
#include <stdio.h>

char* forkNames[] = 
{
	"main",						
	"fsm",						
	"visibilitymap",			
	"init"						
};

const SICommon sCommonHelper = 
{ 
	nextPowerOf2,
    setExecFold,
    getExecFold,
	fillCommonParams
};

const ICommon commonHelper = &sCommonHelper;

Bool	IsPostmaster = False;
Bool	IsSysLogger  = False;
int     ProcId       = -1;

char*        execFolder   = NULL;
char         ExecPath[MAX_PATH];
char*        DataDir      = NULL;
socket_type  ListenSockets[MAX_LISTEN];
long         CancelKey    = 0;
int          ChildSlot    = 0;
int64        StartTime    = 0;
int64        ReloadTime   = 0;
int64        LoggerFileTime = 0;
Bool		 RedirectDone   = False;

/* Maximum number of backend child processes */
int          MaxBackends  = 1000; 

/* This function computes the smallest power of 2 number
 * which is more than 'num'. 
 * For example: num = 67. 
 * The left neighbour is 64, the right neighbour is 128.
 */
int nextPowerOf2(long num)
{
	int			i;
	long		limit;

	for (i = 0, limit = 1; limit < num; i++, limit <<= 1)
		;
	return 1 << i;
}

void setExecFold(char* fold)
{
    execFolder = fold;
}

char* getExecFold()
{
    return execFolder;
}

void fillCommonParams(int argc, char* argv[])
{
    strcpy(ExecPath, argv[0]);
    DataDir = "Data";
}

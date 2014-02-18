
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
    getExecFold
};

const ICommon commonHelper = &sCommonHelper;

Bool	IsPostmaster = False;
Bool	IsSysLogger  = False;
int     ProcId       = -1;

char* execFolder = NULL;

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


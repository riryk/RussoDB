
#include "tranlog.h"
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

char* GenerateFilePath(struct RelationFileInfo relFileInfo, int backend, int fork)
{
	char* filePath;
	int filePathLength;

    if (relFileInfo.tableSpaceId ==  1)//TABLE_SPACE_GLOBAL_ID)
    {
        filePathLength = 7 + 10 + 1 + 5 + 1;
		filePath = (char*)malloc(filePathLength);
		if (fork != 2) //MAIN_FORK)
			_snprintf_s(
			         filePath, 
					 filePathLength, 
					 "GlobalTableSpace/%u_%s",
					 relFileInfo.relationId, 
					 forkNames[fork]);
		else
			_snprintf_s(filePath, filePathLength, "GlobalTableSpace/%u", relFileInfo.relationId);
    }
	else if (relFileInfo.tableSpaceId == 1) //TABLE_SPACE_DEFAULT_ID)
	{
		/* The default tablespace is {datadir}/base */
		if (backend == -1)
		{
			filePathLength = 5 + 10 + 1 + 10 + 1 + 5 + 1;
			filePath = (char*)malloc(filePathLength);
			if (fork != 2)// MAIN_FORK)
				_snprintf_s(filePath, filePathLength, "DefaultTableSpace/%u/%u_%s",
						 relFileInfo.databaseId, 
						 relFileInfo.relationId,
						 forkNames[fork]);
			else
				_snprintf_s(filePath, filePathLength, "DefaultTableSpace/%u/%u",
						 relFileInfo.databaseId, 
						 relFileInfo.relationId);
		}
		else
		{
			/* OIDCHARS will suffice for an integer, too */
			filePathLength = 5 + 10 + 2 + 10 + 1 + 10 + 1 + 5 + 1;
			filePath = (char*)malloc(filePathLength);
			if (fork != 2) //MAIN_FORK)
				_snprintf_s(filePath, filePathLength, "DefaultTableSpace/%u/t%d_%u_%s",
						 relFileInfo.databaseId, backend, relFileInfo.relationId,
						 forkNames[fork]);
			else
				_snprintf_s(filePath, filePathLength, "DefaultTableSpace/%u/t%d_%u",
						 relFileInfo.databaseId, backend, relFileInfo.relationId);
		}
	}
	else
	{
		/* All other tablespaces are accessed via symlinks */
		if (backend == -1)
		{
			filePathLength = 9 + 1 + 10 + 1
				+ 1 + 10 + 1
				+ 10 + 1 + 5 + 1;
			filePath = (char*)malloc(filePathLength);
			if (fork != 2) // MAIN_FORK)
				_snprintf_s(filePath, filePathLength, "TableSpaces/%s/%u/%u_%s",
				relFileInfo.tableSpaceId, 
						 relFileInfo.databaseId, 
						 relFileInfo.relationId,
						 forkNames[fork]);
			else
				_snprintf_s(filePath, filePathLength, "TableSpaces/%s/%u/%u",
						 relFileInfo.tableSpaceId,
						 relFileInfo.databaseId, 
						 relFileInfo.relationId);
		}
		else
		{
			filePathLength = 9 + 1 + 10 + 1
				+ 1 + 10 + 2
				+ 10 + 1 + 10 + 1 + 5 + 1;
			filePath = (char*)malloc(filePathLength);
			if (fork != 1) //MAIN_FORKNUM)
				_snprintf_s(filePath, filePathLength, "TableSpaces/%s/%u/t%d_%u_%s",
						 relFileInfo.tableSpaceId, 
						 relFileInfo.databaseId, 
						 backend, 
						 relFileInfo.relationId,
						 forkNames[fork]);
			else
				_snprintf_s(filePath, filePathLength, "TableSpaces/%s/%u/t%d_%u",
						 relFileInfo.tableSpaceId, 
						 relFileInfo.databaseId, 
						 backend, 
						 relFileInfo.relationId);
		}
	}
}

char* forkNames[] = 
{
	"main",						
	"fsm",						
	"visibilitymap",			
	"init"						
};

char* GenerateFilePath(RelationFileInfo relFileInfo, int backend, int fork)
{
	char* filePath;
	int filePathLength;

    if (relFileInfo.tableSpaceId == TABLE_SPACE_GLOBAL_ID)
    {
        filePathLength = 7 + 10 + 1 + 5 + 1;
		filePath = (char*)malloc(filePathLength);
		if (fork != MAIN_FORK)
			_snprintf_s(
			         filePath, 
					 filePathLength, 
					 "GlobalTableSpace/%u_%s",
					 relFileInfo.relationId, 
					 forkNames[fork]);
		else
			_snprintf_s(filePath, filePathLength, "GlobalTableSpace/%u", relFileInfo.relationId);
    }
	else if (rnode.spcNode == TABLE_SPACE_DEFAULT_ID)
	{
		/* The default tablespace is {datadir}/base */
		if (backend == -1)
		{
			filePathLength = 5 + 10 + 1 + 10 + 1 + 5 + 1;
			filePath = (char*)malloc(filePathLength);
			if (fork != MAIN_FORK)
				snprintf(filePath, filePathLength, "DefaultTableSpace/%u/%u_%s",
						 relFileInfo.databaseId, 
						 relFileInfo.relationId,
						 forkNames[forknum]);
			else
				snprintf(filePath, filePathLength, "DefaultTableSpace/%u/%u",
						 relFileInfo.databaseId, 
						 relFileInfo.relationId);
		}
		else
		{
			/* OIDCHARS will suffice for an integer, too */
			filePathLength = 5 + 10 + 2 + 10 + 1 + 10 + 1 + 5 + 1;
			filePath = (char*)malloc(filePathLength);
			if (fork != MAIN_FORK)
				snprintf(filePath, filePathLength, "DefaultTableSpace/%u/t%d_%u_%s",
						 relFileInfo.databaseId, backend, relFileInfo.relationId,
						 forkNames[fork]);
			else
				snprintf(filePath, filePathLength, "DefaultTableSpace/%u/t%d_%u",
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
			if (fork != MAIN_FORK)
				snprintf(filePath, filePathLength, "TableSpaces/%s/%u/%u_%s",
						 relFileInfo.tablespaceId, 
						 relFileInfo.databaseId, 
						 relFileInfo.relationId,
						 forkNames[fork]);
			else
				snprintf(filePath, filePathLength, "TableSpaces/%s/%u/%u",
						 relFileInfo.tablespaceId,
						 relFileInfo.databaseId, 
						 relFileInfo.relationId);
		}
		else
		{
			filePathLength = 9 + 1 + 10 + 1
				+ 1 + 10 + 2
				+ 10 + 1 + 10 + 1 + 5 + 1;
			filePath = (char*)malloc(filePathLength);
			if (forknum != MAIN_FORKNUM)
				snprintf(filePath, filePathLength, "TableSpaces/%s/%u/t%d_%u_%s",
						 relFileInfo.tablespaceId, 
						 relFileInfo.databaseId, 
						 backend, 
						 relFileInfo.relationId,
						 forkNames[forknum]);
			else
				snprintf(filePath, filePathLength, "TableSpaces/%s/%u/t%d_%u",
						 relFileInfo.tablespaceId, 
						 relFileInfo.databaseId, 
						 backend, 
						 relFileInfo.relationId);
		}
	}
}
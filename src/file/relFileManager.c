
#include "relfilemanager.h"

char* getFilePath(SRelFileInfo relFile, int backend, int part)
{
	char*    path;
	int      pathLen;

	/* table space id 1 means a global space. */
	if (relFile.tblSpaceId == GLOBAL_TBL_SPACE)
	{
		pathLen = 24;
		path    = (char*)malloc(pathLen);

		if (part != FILE_PART_MAIN)
		{
            _snprintf_s(path, pathLen, "GlobalTableSpace/%u_%s", relFile.relId, forkNames[part]);
			return path;
		}

		_snprintf_s(path, pathlen, "GlobalTableSpace/%u", relFile.relId);
		return path;
	}

	if (relFile.tblSpaceId == DEFAULT_TBL_SPACE)
	{
		if (backend == INVALID_BACK_ID)
		{
            pathLen  = 33;
			path     = (char*)malloc(pathLen);

			if (part != FILE_PART_MAIN)
			{
				_snprintf_s(path, pathLen, "DefaultTableSpace/%u/%u_%s", relFile.databaseId, relFile.relId, forkNames[part]);
                return path;
			}

		    _snprintf_s(path, pathLen, "DefaultTableSpace/%u/%u", relFile.databaseId, relFile.relId);
			return path;
		}

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

FileSeg fileRelOpen(RelData rel, FilePartNumber partnum, ExtensionBehavior behavior)
{
    /* if partnum-th part is not null, that means that
	 * this part is already open. We do nothing in this case and 
	 * simply return this file handler.
	 */
	if (rel->parts[partnum])
        return rel->parts[partnum];

    path = relpath(reln->smgr_rnode, forknum);
}

int getBlocksNum(RelData rel, FilePartNumber partnum)
{
    MdfdVec    *v = mdopen(reln, forknum, EXTENSION_FAIL);      
}
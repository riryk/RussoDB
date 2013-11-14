
#include "relfilemanager.h"

const SIRelFileManager sRelFileManager = 
{ 
	&sFileManager,
	&sTrackMemManager,
    getBlocksNum
};

const IRelFileManager relFileManager = &sRelFileManager;

const char* filePartNames[] = 
{
	"main",						/* FILE_PART_MAIN */
	"fs",						/* FILE_PART_FILE_STORAGE */
	"vm",						/* FILE_PART_VISIBILITY_MAP */
	"init"						/* FILE_PART_INIT */
};

char* getFilePath(
    void*         self,
	SRelFileInfo  relFile, 
	int           backend, 
	int           part)
{
	IRelFileManager  _ = (IRelFileManager)self;
	char*            path;
	int              pathLen;

	/* table space id 1 means a global space. */
	if (relFile.tblSpaceId == GLOBAL_TBL_SPACE)
	{
		pathLen = 24;
		path    = (char*)_->memManager->alloc(pathLen);

		if (part != FILE_PART_MAIN)
		{
            _snprintf_s(path, pathLen, "GlobalTableSpace/%u_%s", 
				        relFile.relId, filePartNames[part]);
			return path;
		}

		_snprintf_s(path, pathLen, "GlobalTableSpace/%u", 
			        relFile.relId);
		return path;
	}

	if (relFile.tblSpaceId == DEFAULT_TBL_SPACE)
	{
		if (backend == INVALID_BACK_ID)
		{
            pathLen  = 33;
			path     = (char*)_->memManager->alloc(pathLen);

			if (part != FILE_PART_MAIN)
			{
				_snprintf_s(path, pathLen, "DefaultTableSpace/%u/%u_%s", 
					        relFile.databaseId, relFile.relId, filePartNames[part]);
                return path;
			}

		    _snprintf_s(path, pathLen, "DefaultTableSpace/%u/%u", 
				        relFile.databaseId, relFile.relId);
			return path;
		}

	    pathLen   = 45;
	    path      = (char*)_->memManager->alloc(pathLen);

		if (part != FILE_PART_MAIN)
		{
		    _snprintf_s(path, pathLen, "DefaultTableSpace/%u/t%d_%u_%s",
				        relFile.databaseId, backend, relFile.relId, filePartNames[part]);
			return path;
		}

		_snprintf_s(path, pathLen, "DefaultTableSpace/%u/t%d_%u",
			        relFile.databaseId, backend, relFile.relId);
		return path;
	}

	if (backend == -1)
	{
		pathLen  = 50;
		path     = (char*)_->memManager->alloc(pathLen);

		if (part != FILE_PART_MAIN)
		{
			_snprintf_s(path, pathLen, "TableSpaces/%s/%u/%u_%s",
			            relFile.tblSpaceId, relFile.databaseId, 
						relFile.relId, filePartNames[part]);
            return path;
		}

		_snprintf_s(path, pathLen, "TableSpaces/%s/%u/%u",
			        relFile.tblSpaceId, relFile.databaseId, 
					relFile.relId);
        return path;
	}
		
	pathLen  = 62;
	path     = (char*)_->memManager->alloc(pathLen);

	if (part != FILE_PART_MAIN)
    { 
	   _snprintf_s(path, pathLen, "TableSpaces/%s/%u/t%d_%u_%s",
		           relFile.tblSpaceId, relFile.databaseId, 
				   backend, relFile.relId, filePartNames[part]);
       return path;
	}
       
	_snprintf_s(path, pathLen, "TableSpaces/%s/%u/t%d_%u",
		        relFile.tblSpaceId, relFile.databaseId, 
				backend, relFile.relId);
	return path;
}

FileSeg fileRelOpen(
    void*              self,
	RelData            rel, 
	FilePartNumber     partnum)
{
	IRelFileManager  _ = (IRelFileManager)self;
	FileSeg          fileSeg;
    char*            path;
	int              fileDesc;
    /* if partnum-th part is not null, that means that
	 * this part is already open. We do nothing in this case and 
	 * simply return this file handler.
	 */
	if (rel->parts[partnum])
        return rel->parts[partnum];

    path     = getFilePath(_, rel->relKey.node, rel->relKey.backend, partnum);
	fileDesc = _->fileManager->openFile(self, path, O_RDWR | O_BINARY, 0600);

	_->memManager->free(path);

	fileSeg = (FileSeg)_->memManager->alloc(sizeof(SFileSeg));
	rel->parts[partnum] = fileSeg;

	fileSeg->fileDesc = fileDesc;
	fileSeg->segNum   = 0;
	fileSeg->segNext  = NULL;
}

FileSeg openRelSegm(
    void*              self,
	RelData            rel, 
	FilePartNumber     partnum,
	uint               segnum,
	int                flags)
{
	IRelFileManager  _   = (IRelFileManager)self;
	FileSeg          fileSeg;
	char*            path = getFilePath(
		                 _,
	                     rel->relKey.node, 
	                     rel->relKey.backend, 
	                     partnum);

	int              fileDesc = _->fileManager->openFile(
		                 self,
		                 path, 
						 O_RDWR | O_BINARY | flags, 
						 0600);

    _->memManager->free(path);
	if (fileDesc < 0)
		return NULL;

    fileSeg = (FileSeg)_->memManager->alloc(sizeof(SFileSeg));

	fileSeg->fileDesc = fileDesc;
	fileSeg->segNum   = segnum;
	fileSeg->segNext  = NULL;

	return fileSeg;
} 

int calculateBlocks(
    void*            self,
	RelData          rel,
	FilePartNumber   partnum,
	FileSeg          seg)
{
    IRelFileManager  _   = (IRelFileManager)self;
	long             len = _->fileManager->restoreFilePos(seg->fileDesc, 0, SEEK_END);
	if (len < 0)
		;// We need to report an error
    return len / BLOCK_SIZE;
}

int getBlocksNum(
    void*            self,
	RelData          rel, 
	FilePartNumber   partnum)
{
	FileSeg          fileSeg = fileRelOpen(self, rel, partnum);
	uint             blocksCount;
    uint             segmNum;

	/* We have (n-1) full segments in a relation and  */
	while (fileSeg->segNext != NULL)
	{
		segmNum++;
		fileSeg = fileSeg->segNext;
	}

    blocksCount = calculateBlocks(self, rel, partnum, fileSeg);
	if (blocksCount < REL_SEGM_SIZE)
        return segmNum * REL_SEGM_SIZE + blocksCount;
        
    /* If the last segment is full, we need to create another one. */
    segmNum++;
	
    /* By using O_CREAT flag we create a new segment file of size 0 */
	fileSeg->segNext = openRelSegm(self, rel, partnum, segmNum, O_CREAT);
    blocksCount = calculateBlocks(self, rel, partnum, fileSeg);

    return segmNum * REL_SEGM_SIZE + blocksCount;
}
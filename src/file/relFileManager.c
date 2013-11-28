
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

void ctorRelFileMan(void* self)
{
    IRelFileManager  _  = (IRelFileManager)self;
	IFileManager     fm = _->fileManager;

	fm->ctorFileMan(fm);
}

void createRelPart(
    void*         self,
	RelData       rel, 
	int           pnum)
{
	IRelFileManager _  = (IRelFileManager)self;
    IFileManager    fm = _->fileManager;
    IMemoryManager  mm = _->memManager;

    FileSeg*        pt  = &(rel->parts[pnum]); 
    char*           p;
    int             fd;    
	RelFileInfoBack key = &(rel->relKey);

    if (*pt != NULL)
		return;

    p  = _->getFilePath(_, &(key->node), key->backend, pnum);
	fd = fm->openFile(_, p, O_RDWR | O_CREAT | O_EXCL | O_BINARY, 600);

    mm->free(p);

	*pt = (FileSeg)mm->alloc(sizeof(SFileSeg));

	(*pt)->fileDesc = fd;
	(*pt)->segNum   = 0;
	(*pt)->segNext  = NULL;

	return pt;
}

char* getFilePath(
    void*         self,
	RelFileInfo   relFile, 
	int           backend, 
	int           part)
{
	IRelFileManager  _ = (IRelFileManager)self;
    IMemoryManager   mm = _->memManager;

	char*            p;
	int              len;
    char**           names = filePartNames; 

	if (relFile->tblSpaceId == GLOBAL_TBL_SPACE)
	{
		len = sizeof(TBL_SPACE_GLOBAL) 
			+ 1                        /* for '/' symbol  */
			+ MAX_PRINTED_CHARS        /* For relation id */
			+ 1                        /* for '_' symbol  */
			+ REL_PART_LEN;            /* for relation part */

		p = (char*)mm->alloc(len);

		if (part != FILE_PART_MAIN)
            _snprintf_s(p, 
			            len, 
			            "%s/%u_%s", 
			            TBL_SPACE_GLOBAL, 
						relFile->relId,
						names[part]);
		else
            _snprintf_s(p, 
			            len, 
			            "%s/%u", 
			            TBL_SPACE_GLOBAL, 
						relFile->relId);

		return p;
	}

	if (relFile->tblSpaceId == DEFAULT_TBL_SPACE)
	{
		if (backend == INVALID_BACK_ID)
		{
            len = sizeof(TBL_SPACE_DEFAULT)
				+ 1                     /* for '/' symbol  */
			    + MAX_PRINTED_CHARS     /* for database id */
                + 1                     /* for '/' symbol  */ 
				+ MAX_PRINTED_CHARS     /* for relation id */
                + 1                     /* for '_' symbol  */
			    + REL_PART_LEN;         /* for relation part */ 

			p  = (char*)mm->alloc(len);

			if (part != FILE_PART_MAIN)
				_snprintf_s(p, 
					        len, 
							"%s/%u/%u_%s", 
							TBL_SPACE_DEFAULT,
					        relFile->databaseId, 
							relFile->relId, 
							names[part]);
			else
		        _snprintf_s(p, 
				            len, 
							"%s/%u/%u", 
                            TBL_SPACE_DEFAULT,
				            relFile->databaseId, 
							relFile->relId);
			return p;
		}

		len = sizeof(TBL_SPACE_DEFAULT)
				+ 1                     /* for '/' symbol  */
			    + MAX_PRINTED_CHARS     /* for database id */
                + 1                     /* for '/' symbol  */ 
				+ 1                     /* for 't' symbol  */
				+ MAX_PRINTED_CHARS     /* for backend     */ 
				+ MAX_PRINTED_CHARS     /* for relation id */
                + 1                     /* for '_' symbol  */
			    + REL_PART_LEN;         /* for relation part */ 

	    p = (char*)mm->alloc(len);

		if (part != FILE_PART_MAIN)
		    _snprintf_s(p, 
				        len, 
						"%s/%u/t%d_%u_%s",
                        TBL_SPACE_DEFAULT,
				        relFile->databaseId, 
						backend, 
						relFile->relId, 
						names[part]);
		else
		    _snprintf_s(p, 
			            len, 
				   	    "%s/%u/t%d_%u",
			            relFile->databaseId, 
					    backend, 
					    relFile->relId);
		return p;
	}

	if (backend == INVALID_BACK_ID)
	{
        len = sizeof(TBL_SPACES)
				+ 1                     /* for '/' symbol  */
			    + MAX_PRINTED_CHARS     /* for tblSpace Id */
                + 1                     /* for '/' symbol  */ 
				+ MAX_PRINTED_CHARS     /* for database Id */ 
				+ 1                     /* for '/' symbol */
				+ MAX_PRINTED_CHARS     /* for relation id */
                + 1                     /* for '_' symbol  */
			    + REL_PART_LEN;         /* for relation part */ 

		p   = (char*)mm->alloc(len);

		if (part != FILE_PART_MAIN)
			_snprintf_s(p, 
				        len, 
						"%s/%s/%u/%u_%s",
						TBL_SPACES,
						relFile->tblSpaceId, 
						relFile->databaseId, 
						relFile->relId, 
						names[part]);
		else
		    _snprintf_s(p, 
			            len, 
				  	    "%s/%s/%u/%u",
                        TBL_SPACES,
			            relFile->tblSpaceId, 
					    relFile->databaseId, 
					    relFile->relId);

        return p;
	}

    len = sizeof(TBL_SPACES)
		     + 1                     /* for '/' symbol  */
			 + MAX_PRINTED_CHARS     /* for tblSpace Id */
             + 1                     /* for '/' symbol  */ 
			 + MAX_PRINTED_CHARS     /* for database Id */ 
			 + 1                     /* for '/' symbol  */ 
			 + MAX_PRINTED_CHARS     /* for backend     */ 
			 + 1                     /* for '/' symbol  */
			 + MAX_PRINTED_CHARS     /* for relation id */
             + 1                     /* for '_' symbol  */
			 + REL_PART_LEN;         /* for relation part */ 

	p   = (char*)mm->alloc(len);

	if (part != FILE_PART_MAIN)
	   _snprintf_s(p, 
		           len, 
				   "%s/%s/%u/t%d_%u_%s",
                   TBL_SPACES,
				   relFile->tblSpaceId,
				   relFile->databaseId,
				   backend, 
				   relFile->relId, 
				   names[part]);
	else
	   _snprintf_s(p, 
	  	           len, 
				   "%s/%s/%u/t%d_%u",
                   TBL_SPACES,
		           relFile->tblSpaceId, 
				   relFile->databaseId, 
				   backend, 
				   relFile->relId);

	return p;
}

FileSeg openRel(
    void*              self,
	RelData            rel, 
	FilePartNumber     pnum)
{
	IRelFileManager   _  = (IRelFileManager)self;
	IFileManager      fm = _->fileManager;
	IMemoryManager    mm = _->memManager;

	FileSeg           seg;
    char*             p;
	int               fd;
	RelFileInfoBack   key = &(rel->relKey);
	FileSeg*          pt  = &(rel->parts[pnum]);            

    /* if partnum-th part is not null, that means that
	 * this part is already open. We do nothing in this case and 
	 * simply return this file handler.
	 */
	if (*pt)
        return *pt;

    p  = _->getFilePath(_, &(key->node), key->backend, pnum);
	fd = fm->openFile(_, p, O_RDWR | O_BINARY, 600);

	mm->free(p);

	*pt = seg = (FileSeg)mm->alloc(sizeof(SFileSeg));

	seg->fileDesc = fd;
	seg->segNum   = 0;
	seg->segNext  = NULL;

	return pt;
}

FileSeg openRelSegm(
    void*              self,
	RelData            rel, 
	FilePartNumber     part,
	uint               segnum,
	int                flags)
{
	IRelFileManager   _  = (IRelFileManager)self;
    IFileManager      fm = _->fileManager;
	IMemoryManager    mm = _->memManager;

	FileSeg           seg;
    RelFileInfoBack   key = &(rel->relKey);

    char*             p;
    int               fd;

	p = _->getFilePath(_, &(key->node), key->backend, part);
	fd = fm->openFile(_, p, O_RDWR | O_BINARY | flags, 600);

    mm->free(p);
	if (fd < 0)
		return NULL;

    seg = (FileSeg)mm->alloc(sizeof(SFileSeg));

	seg->fileDesc = fd;
	seg->segNum   = segnum;
	seg->segNext  = NULL;

	return seg;
} 

int calculateBlocks(
    void*            self,
	RelData          rel,
	FilePartNumber   partnum,
	FileSeg          seg)
{
    IRelFileManager  _  = (IRelFileManager)self;
    IFileManager     fm = _->fileManager;
	long             len = fm->restoreFilePos(fm, seg->fileDesc, 0, SEEK_END);

    return len / BLOCK_SIZE;
}

int getBlocksNum(
    void*            self,
	RelData          rel, 
	FilePartNumber   partnum)
{
	FileSeg          fileSeg = openRel(self, rel, partnum);
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
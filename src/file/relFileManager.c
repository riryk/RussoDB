
#include "relfilemanager.h"

const SIRelFileManager sRelFileManager = 
{ 
	&sFileManager,
	&sTrackMemManager,
    getBlocksNum
};

const IRelFileManager relFileManager = &sRelFileManager;

Hashtable     pendingOpsTable;


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

FileSeg createRelPart(
    void*         self,
	char*         execfold,
	RelData       rel, 
	int           pnum)
{
	IRelFileManager _  = (IRelFileManager)self;
    IFileManager    fm = _->fileManager;
    IMemoryManager  mm = _->memManager;

    FileSeg*        pt  = &(rel->parts[pnum]); 
    char*           p;
    int             find;    
	RelFileInfoBack key = &(rel->relKey);

    if (*pt != NULL)
		return;

    p    = _->getFilePath(_, execfold, &(key->node), key->backend, pnum);
	find = fm->openFileToCache(fm, p, _O_CREAT, O_BINARY | O_RDWR | O_EXCL);

	*pt  = (FileSeg)mm->alloc(sizeof(SFileSeg));

	(*pt)->find     = find;
	(*pt)->num      = 0;
	(*pt)->next     = NULL;
	(*pt)->fname    = p;

	return *pt;
}

char* getFilePath(
    void*         self,
	char*         execfold,
	RelFileInfo   relFile, 
	int           backend, 
	int           part)
{
	IRelFileManager  _ = (IRelFileManager)self;
    IMemoryManager   mm = _->memManager;

	char*            p;
	char*            start    = "";
	int              startlen = 0;
	int              len;
    char**           names    = filePartNames; 

	if (execfold != NULL)
	{
		startlen = strlen(execfold) 
                 + 1;           /* for '/' symbol */

        snprintf(start, 
		         startlen, 
		         "%s/", 
		         execfold);
	}

	if (relFile->tblSpaceId == GLOBAL_TBL_SPACE)
	{
		len = strlen(TBL_SPACE_GLOBAL) 
			+ 1                        /* for '/' symbol  */
			+ MAX_PRINTED_CHARS        /* For relation id */
			+ 1                        /* for '_' symbol  */
			+ REL_PART_LEN             /* for relation part */
            + startlen; 

		p = (char*)mm->alloc(len);

		if (part != FILE_PART_MAIN)
            snprintf(p, 
		             len, 
		             "%s%s/%u_%s", 
                     start, 
		             TBL_SPACE_GLOBAL, 
					 relFile->relId,
					 names[part]);
		else
            snprintf(p, 
		             len, 
		             "%s%s/%u", 
					 start,
		             TBL_SPACE_GLOBAL, 
				 	 relFile->relId);

		return p;
	}

	if (relFile->tblSpaceId == DEFAULT_TBL_SPACE)
	{
		if (backend == INVALID_BACK_ID)
		{
            len = strlen(TBL_SPACE_DEFAULT)
				+ 1                     /* for '/' symbol  */
			    + MAX_PRINTED_CHARS     /* for database id */
                + 1                     /* for '/' symbol  */ 
				+ MAX_PRINTED_CHARS     /* for relation id */
                + 1                     /* for '_' symbol  */
			    + REL_PART_LEN          /* for relation part */ 
			    + startlen; 

			p  = (char*)mm->alloc(len);

			if (part != FILE_PART_MAIN)
				snprintf(p, 
				         len, 
						 "%s%s/%u/%u_%s", 
						 start,
						 TBL_SPACE_DEFAULT,
				         relFile->databaseId, 
						 relFile->relId, 
						 names[part]);
			else
		        snprintf(p, 
			             len, 
						 "%s%s/%u/%u", 
						 start,
                         TBL_SPACE_DEFAULT,
			             relFile->databaseId, 
						 relFile->relId);
			return p;
		}

		len = strlen(TBL_SPACE_DEFAULT)
				+ 1                     /* for '/' symbol  */
			    + MAX_PRINTED_CHARS     /* for database id */
                + 1                     /* for '/' symbol  */ 
				+ 1                     /* for 't' symbol  */
				+ MAX_PRINTED_CHARS     /* for backend     */ 
				+ MAX_PRINTED_CHARS     /* for relation id */
                + 1                     /* for '_' symbol  */
			    + REL_PART_LEN          /* for relation part */ 
                + startlen; 

	    p = (char*)mm->alloc(len);

		if (part != FILE_PART_MAIN)
		    snprintf(p, 
			         len, 
					 "%s%s/%u/t%d_%u_%s",
					 start,
                     TBL_SPACE_DEFAULT,
			         relFile->databaseId, 
					 backend, 
					 relFile->relId, 
					 names[part]);
		else
		    snprintf(p, 
		             len, 
			   	     "%s%s/%u/t%d_%u",
					 start,
		             relFile->databaseId, 
				     backend, 
				     relFile->relId);
		return p; 
	}

	if (backend == INVALID_BACK_ID)
	{
        len = strlen(TBL_SPACES)
				+ 1                     /* for '/' symbol  */
			    + MAX_PRINTED_CHARS     /* for tblSpace Id */
                + 1                     /* for '/' symbol  */ 
				+ MAX_PRINTED_CHARS     /* for database Id */ 
				+ 1                     /* for '/' symbol */
				+ MAX_PRINTED_CHARS     /* for relation id */
                + 1                     /* for '_' symbol  */
			    + REL_PART_LEN          /* for relation part */ 
                + startlen; 

		p   = (char*)mm->alloc(len);

		if (part != FILE_PART_MAIN)
			snprintf(p, 
			         len, 
					 "%s%s/%s/%u/%u_%s",
					 start,
					 TBL_SPACES,
					 relFile->tblSpaceId, 
					 relFile->databaseId, 
					 relFile->relId, 
					 names[part]);
		else
		    snprintf(p, 
		             len, 
			  	     "%s%s/%s/%u/%u",
					 start,
                     TBL_SPACES,
		             relFile->tblSpaceId, 
				     relFile->databaseId, 
				     relFile->relId);

        return p;
	}

    len = strlen(TBL_SPACES)
		     + 1                     /* for '/' symbol  */
			 + MAX_PRINTED_CHARS     /* for tblSpace Id */
             + 1                     /* for '/' symbol  */ 
			 + MAX_PRINTED_CHARS     /* for database Id */ 
			 + 1                     /* for '/' symbol  */ 
			 + MAX_PRINTED_CHARS     /* for backend     */ 
			 + 1                     /* for '/' symbol  */
			 + MAX_PRINTED_CHARS     /* for relation id */
             + 1                     /* for '_' symbol  */
			 + REL_PART_LEN          /* for relation part */ 
	         + startlen; 

	p   = (char*)mm->alloc(len);

	if (part != FILE_PART_MAIN)
	   snprintf(p, 
	            len, 
			    "%s%s/%s/%u/t%d_%u_%s",
				start,
                TBL_SPACES,
			    relFile->tblSpaceId,
			    relFile->databaseId,
			    backend, 
			    relFile->relId, 
			    names[part]);
	else
	   snprintf(p, 
  	            len, 
			    "%s%s/%s/%u/t%d_%u",
				start,
                TBL_SPACES,
	            relFile->tblSpaceId, 
			    relFile->databaseId, 
			    backend, 
			    relFile->relId);

	return p;
}

FileSeg openRel(
    void*              self,
	char*              execfold,
	RelData            rel, 
	FilePartNumber     pnum)
{
	IRelFileManager   _  = (IRelFileManager)self;
	FileSeg*          pt  = &(rel->parts[pnum]);            

    /* if partnum-th part is not null, that means that
	 * this part is already open. We do nothing in this case and 
	 * simply return this file handler.
	 */
	if (*pt)
        return *pt;

    return _->createRelPart(_, execfold, rel, pnum);
}

FileSeg openRelSegm(
    void*              self,
	char*              execfold,
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
	char*             pold;
    int               find;
	int               seglen;

	p  =  _->getFilePath(_, execfold, &(key->node), key->backend, part);

	if (segnum > 0)
	{
		pold   = p;
		seglen = strlen(p) + REL_SEGM_LEN;
		p      = (char*)mm->alloc(seglen); 
        snprintf(p, seglen, "%s.%u", pold, segnum);
        mm->free(pold);    
	}

	find = fm->openFileToCache(fm, p, flags | O_RDWR, O_BINARY);

	if (find < 0)
		return NULL;

    seg = (FileSeg)mm->alloc(sizeof(SFileSeg));

	seg->find  = find;
	seg->num   = segnum;
	seg->next  = NULL;
	seg->fname = p;

	return seg;
} 

void closeSegm(
    void*             self,
    FileSeg           seg)
{
    IRelFileManager  _      = (IRelFileManager)self;
    IFileManager     fm     = _->fileManager;

	fm->deleteFileFromCache(fm, seg->find);
}

int getBlocksNum(
    void*            self,
    char*            fold,
	RelData          rel, 
	FilePartNumber   part,
	int              segmsize)
{
    IRelFileManager  _      = (IRelFileManager)self;
    IFileManager     fm     = _->fileManager;

	FileSeg          seg    = _->openRel(_, fold, rel, part);
	uint             bnum;
    uint             snum   = 0;
	long             len;

	/* We have (n-1) full segments in a relation and 
	 * it is pointless to calculate blocks number in
	 * filles segments because we know it exactly.
	 */
	while (seg->next != NULL)
	{
		snum++;
		seg = seg->next;
	}
    
	/* Here seg points to the last segment. 
	 * We need to lseek in this file to the end
	 * to calculate the size of the file.
	 */
	len = fm->restoreFilePos(fm, seg->find, 0, SEEK_END);

	/* And calculate the number of blocks. */
	bnum = len / BLOCK_SIZE;

	/* If the last segment is not filled,
	 * we simply return the total blocks number.
	 * If the last segment is completely filled,
	 * we need to allocate a new one.
	 */
	if (bnum < segmsize)
        return snum * segmsize + bnum;
        
    /* If the last segment is full, we need to create another one. */
    snum++;
	
    /* By using O_CREAT flag we create a new segment file of size 0 */
	seg->next = _->openRelSegm(_, fold, rel, part, snum, O_CREAT);

    return snum * segmsize;
}

void addblock(
    void*              self,
	char*              fold,
	RelData            rel,  
	FilePartNumber     part,
	uint               block,
    char*              buffer, 
	Bool               skipFsync)
{
	FileSeg          seg;

    //seg = _mdfd_getseg(reln, forknum, blocknum, skipFsync, EXTENSION_CREATE);
}

FileSeg findBlockSegm(
    void*              self,
	char*              fold,
	RelData            rel, 	
	FilePartNumber     part,
	uint               block,
	ExtensionBehavior  behavior,
	int                segmsize)
{
	IRelFileManager  _      = (IRelFileManager)self;
    IFileManager     fm     = _->fileManager;
    IMemoryManager   mm     = _->memManager;

    uint             segnum;
	uint             i;
	FileSeg          seg    = _->openRel(_, fold, rel, part);
	int              len, bnum;

    if (seg == NULL)
		return NULL;
    
    segnum = block / segmsize;

	/* Loop through all segments before our current segment
	 * and open all these segments and build a one-directional list.
	 */
    for (i = 1; i <= segnum; i++, seg = seg->next)
	{
		if (seg->next != NULL)
			continue;

        /* Here seg->next is null. We need to open it.
		 * We just open the next relation segment and 
		 */
        if (behavior != EXTENSION_CREATE)
		{
            seg->next = _->openRelSegm(_, fold, rel, part, i, 0);
			continue;
		}

		seg->next = _->openRelSegm(_, fold, rel, part, i, O_CREAT);
	}
	return seg;
}

void writeBlock(
    void*            self,
    char*            fold,
	RelData          rel, 	
	FilePartNumber   part,
    uint             block,
	char*            buffer,
	Bool             skipFsync)
{
	IRelFileManager  _      = (IRelFileManager)self;
    IFileManager     fm     = _->fileManager;

    FileSeg          seg;
	off_t		     seekpos;
	int              nbytes;

	seg     = _->findBlockSegm(_, fold, rel, part, block, skipFsync, EXTENSION_FAIL);
	seekpos = (off_t)BLOCK_SIZE *(block % REL_SEGM_LEN);

	if (fm->restoreFilePos(fm, seg->find, seekpos, SEEK_END) != seekpos)
        ; //report an error

    nbytes = FileWrite(seg->find, buffer, BLOCK_SIZE);
	if (nbytes != BLOCK_SIZE)
	    ; //report an error

	_->pushFSyncRequest(_, fold, rel, part, seg);
}

void pushFSyncRequest(
    void*            self,
    char*            fold,
	RelData          rel, 
	FilePartNumber   part,
	FileSeg          seg)
{
    if (pendingOpsTable)
	{
        /* push it into local pending-ops table */
		//RememberFsyncRequest(reln->smgr_rnode.node, forknum, seg->mdfd_segno);
	}
}
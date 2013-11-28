#include "tranlog.h"
//#include <stat.h>
#include <stdio.h>

static unsigned int timeLineID = 0;

static struct TranLogState* LogState; // = NULL; 
static struct TranLogWriteProgress TranLogProgress  = {{0, 0}, {0, 0}};

static int    openLogFileDescriptor = -1;
static unsigned int openLogId = 0;
static unsigned int openLogSeg = 0;
static unsigned int openLogOffset = 0;

int MustBeBackedUp(
	struct tran_log_data* tranData, 
	int doPageWrites,
	uint64* lsn, 
	struct BackedUpBlock* bkpb)
{
    void*		page;

	page = (void*)GetBlock(tranData->bufferId);
	*lsn = (uint64)(((struct PageHeader*)page)->tranLogId << 32 
		          | ((struct PageHeader*)page)->tranRecordOffset);

	return 0;
} 

int TranLogIdFileInit(uint64 segmentNumber, int* useExistent, int useLock)
{
    char		path[1024];
	char		tmppath[1024];
    int			fileDescriptor;
	char*       zeroBuffer;
	int			i;

	_snprintf_s(
		path, 
		1024, 
		"tran_log/%08X%08X%08X", 
		0, //ThisTimeLineID,	
		(unsigned int)(segmentNumber / TRAN_LOG_SEG_NUMBER),
	    (unsigned int)(segmentNumber % TRAN_LOG_SEG_NUMBER));
     
    if (*useExistent)
	{
        fileDescriptor = FileOpenBase(path, 0);//O_RDWR | PG_BINARY, S_IRUSR | S_IWUSR);
		if (fileDescriptor < 0)
			Log("error");
		else
			return fileDescriptor;
	}

	_snprintf_s(tmppath, 1024, "tran_log/xlogtemp.%d", (int)getpid());

    unlink(tmppath);

	/* do not use get_sync_bit() here --- want to fsync only at end of fill */
	fileDescriptor = FileOpenBase(tmppath, 0);
		               /*O_RDWR | O_CREAT | O_EXCL | PG_BINARY,
					   S_IRUSR | S_IWUSR);*/
	if (fileDescriptor < 0)
        Log("error");

	zeroBuffer = (char*)malloc(TRAN_LOG_BLOCK_SIZE);
	memset(zeroBuffer, 0, TRAN_LOG_BLOCK_SIZE);

	for (i = 0; i < TRAN_LOG_SEG_SIZE; i += TRAN_LOG_BLOCK_SIZE)
	{
        if ((int)write(fileDescriptor, zeroBuffer, TRAN_LOG_BLOCK_SIZE) != (int)TRAN_LOG_BLOCK_SIZE)
		{
			unlink(tmppath);
            Log("error");
		}
	}
	free(zeroBuffer);

	//if (fsync(fileDescriptor) != 0)
    //    Log("error");

	if (close(fileDescriptor))
        Log("error");

	{
        int* log = (int*)(segmentNumber / TRAN_LOG_SEG_NUMBER);
		int* seg = (int*)(segmentNumber % TRAN_LOG_SEG_NUMBER); 
		int     find_free = *useExistent; 
		int*    max_advance = TRAN_LOG_FILES_COUNT;
		int     use_lock = useLock;
		int     canAllocNewSegment = 0;

	    if (!find_free)
		   unlink(path);
	    else
	    {
		   //struct stat    FileStatistic;
           /*while (!stat(path, &FileStatistic))
		   {
               if (*max_advance <= 0)
			      return 0;

			   (*seg)++;
			   (*max_advance)--;

			   snprintf(
		          path, 
		          1024, 
		          "tran_log/%08X%08X%08X", 
		          ThisTimeLineID,	
		          *log,
	              *seg);

			   while (!MoveFileEx(tmppath, path, MOVEFILE_REPLACE_EXISTING))
                  Log("error");
		   }*/
	    }

		canAllocNewSegment = 1;
		if (!canAllocNewSegment)
		   unlink(tmppath);

	    //*use_existent = 0;

	    fileDescriptor = FileOpenBase(path, 0);/* O_RDWR | PG_BINARY, S_IRUSR | S_IWUSR);*/
	    if (fileDescriptor < 0)
		   Log("error");

	    return fileDescriptor;
	}
}

int TranLogLess(struct tran_log_id a, struct tran_log_id b)
{
    return  a.log_id < b.log_id || 
		   (a.log_id == b.log_id 
		 && a.tran_offset < b.tran_offset);
}

void TranLogWrite(struct TranLogWriteProgress writeRequest, int flexible, int tranLogSwitch)
{
	struct TranLogWriteState* Write = &LogState->WriteState;
	int                current = Write->currentIndex;
	int	               start;
	unsigned int 	   startOffset;
	int                ispartialpage;
	int	               useExistentFile = 0;
	int	               pagesCount;
	int                bytesCount;
	int                last;
	int		           lastSegment;
	char*              from;

	struct TranLogProgress* tp = LogState->LogProgress;

	while (tp->WritePosition.log_id < writeRequest.WritePosition.log_id || 
		       (tp->WritePosition.log_id == writeRequest.WritePosition.log_id 
			 && tp->WritePosition.tran_offset < writeRequest.WritePosition.tran_offset))
	{
		tp->WritePosition = LogState->blocks[current];

		ispartialpage = writeRequest.WritePosition.log_id < tp->WritePosition.log_id || 
		       (writeRequest.WritePosition.log_id == tp->WritePosition.log_id 
			 && writeRequest.WritePosition.tran_offset < tp->WritePosition.tran_offset);

	    if (!((tp->WritePosition.log_id == openLogId &&
  	          (tp->WritePosition.tran_offset - 1) / TRAN_LOG_SEG_SIZE == openLogSeg)))
		{
			openLogId = tp->WritePosition.log_id;
            openLogSeg = (tp->WritePosition.tran_offset - 1) / TRAN_LOG_SEG_SIZE;

			/* create/use new log file */
			useExistentFile = 1;
			openLogFileDescriptor = TranLogIdFileInit(openLogSeg, &useExistentFile, 1);

			//openLogFile = XLogFileInit(openLogSeg, &use_existent, true);
			openLogOffset = 0;
		}

		/*if (openLogFile < 0)*/
		{
			char		path[1024];
			openLogSeg = (tp->WritePosition.tran_offset - 1) / TRAN_LOG_SEG_SIZE;

	        /*snprintf(path, 1024, "tran_log/%08X%08X%08X", tli,		
			   (unsigned int) (openLogSeg / TRAN_LOG_SEG_NUMBER),
			   (unsigned int) (openLogSeg % TRAN_LOG_SEG_NUMBER));*/

	        openLogFileDescriptor = FileOpenBase(path, 0); /* O_RDWR | PG_BINARY, S_IRUSR | S_IWUSR);*/
	        if (openLogFileDescriptor < 0)
		        Log("error");

			openLogOffset = 0;
		}

		if (pagesCount == 0)
		{
	        start = current;
			startOffset = (TranLogProgress.WritePosition.tran_offset - TRAN_LOG_BLOCK_SIZE) % TRAN_LOG_SEG_SIZE;
		}
		pagesCount++;

		last = !(TranLogProgress.WritePosition.log_id < writeRequest.WritePosition.log_id ||
			    (TranLogProgress.WritePosition.log_id == writeRequest.WritePosition.log_id 
	          && TranLogProgress.WritePosition.tran_offset < writeRequest.WritePosition.tran_offset));
        
		lastSegment = !ispartialpage &&
			(startOffset + pagesCount * TRAN_LOG_BLOCK_SIZE) >= TRAN_LOG_SEG_SIZE;

		if (last ||
			current == LogState->highestBlock ||
			lastSegment)
		{
			int nextBuffer;

            if (openLogOffset != 0) //startoffset)
			{
				if (lseek(openLogFileDescriptor, startOffset, SEEK_SET) < 0)
					Log("Error");

				//openLogOff = 0; //startoffset;
			}

			from = LogState->pages + start * TRAN_LOG_BLOCK_SIZE;
			bytesCount = pagesCount * TRAN_LOG_BLOCK_SIZE;
			if (write(openLogFileDescriptor, from, bytesCount) != bytesCount)
			    Log("Error");

			/* Update state for write */
			//openLogOff += bytesCount;

			nextBuffer == (current == LogState->highestBlock) ? 0 : current + 1;
			Write->currentIndex = ispartialpage ? current : nextBuffer;

			pagesCount = 0;

			if (lastSegment || (tranLogSwitch && last))
			{
			    //fsync(openLogFileDescriptor);
				TranLogProgress.FlushPosition = TranLogProgress.WritePosition;
			}
		}
        
        if (ispartialpage)
		{
			TranLogProgress.WritePosition = writeRequest.WritePosition;
			break;
		}
		current == (current == LogState->highestBlock) ? 0 : current + 1;
        if (flexible && pagesCount == 0)
			break;
	}

    if (TranLogLess(TranLogProgress.FlushPosition, writeRequest.FlushPosition) &&
		TranLogLess(TranLogProgress.FlushPosition, TranLogProgress.WritePosition))
	{
        //fsync(openLogFileDescriptor);
		TranLogProgress.FlushPosition = TranLogProgress.WritePosition;
	}
}


void TranLogInsert(
	unsigned int rmid, 
	unsigned char info, 
	struct tran_log_data* tranData)
{
	int                  i;
	unsigned int         length;
	unsigned int         freeSpace;
	struct tran_log_data*       tranDataItem;
	unsigned int	     buffers[4];
	int		             buffers_in_use[4];
	struct tran_log_record      tranLogHeader;
	struct TranLogInsertState*  insertState = &LogState->InsertState;
	struct TranLogWriteProgress writeProgress;

	for (i = 0; i < 4; i++)
	{
		buffers[i] = 0;
		buffers_in_use[i] = 0;
	}

	length = 0;
    for (tranDataItem = tranData;;)
	{
        if (tranDataItem->bufferId == 0)
		{
			length += tranDataItem->length;
		}
		else
		{
			for (i = 0; i < 4; i++)
			{
                if (tranDataItem->bufferId == buffers[i])
				{
					/* Buffer already referenced by earlier chain item */
					if (buffers_in_use[i])
					{
						tranDataItem->data = NULL;
						tranDataItem->length = 0;
					}
					else if (tranDataItem->data)
						length += tranDataItem->length;
					break;
				}
				if (buffers[i] == 0)
				{
					buffers[i] = tranDataItem->bufferId;
					if (tranDataItem->data)
						length += tranDataItem->length;
					break;
				}
			}
		}
		if (tranDataItem->next == NULL)
			break;
		tranDataItem = tranDataItem->next;
	}

	freeSpace = TRAN_LOG_BLOCK_SIZE; //- (insertState->currentPosition - (char*)insertState->currentPage);
    if (freeSpace < sizeof(struct tran_log_record))
	{
		int notExceeded;
		int nextIndex = (insertState->currentBlockIndex == LogState->highestBlock) ? 0 : insertState->currentBlockIndex + 1;
		struct tran_log_id oldId = insertState->blocks[nextIndex];
         
	    
		if (oldId.log_id >= TranLogProgress.WritePosition.log_id)
		{
			struct tran_log_id a = oldId;
			struct tran_log_id b = TranLogProgress.WritePosition;

			notExceeded =   (a.log_id < b.log_id 
						 || (a.log_id == b.log_id 
						  && a.tran_offset <= b.tran_offset));

			if (!notExceeded)
			{
				struct tran_log_id finishedPageId = insertState->blocks[insertState->currentBlockIndex];
	            
				volatile struct TranLogState* logStateLocal = LogState;
				SpinLockAcquire(logStateLocal->locker, NULL, 0);

				//if (logStateLocal->LogProgress->WritePosition < finishedPageId.log_id)
				//	logStateLocal->LogProgress->WritePosition = finishedPageId.log_id;

				//TranLogProgress = logStateLocal->LogProgress;

				SpinLockRelease(logStateLocal->locker);
			}

			if (oldId.log_id >= TranLogProgress.WritePosition.log_id)
		    {
				writeProgress.WritePosition = oldId;
				writeProgress.FlushPosition.log_id = 0;
				writeProgress.FlushPosition.tran_offset = 0;
			}
		}
	}
	
}
#include "tranlog.h"

static uint32 timeLineID = 0;

static struct TranLogState* LogState = NULL; 
static struct TranLogProgress  = {{0, 0}, {0, 0}};

static int    openLogFileDescriptor = -1;
static uint32 openLogId = 0;
static uint32 openLogSeg = 0;


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

	snprintf(
		path, 
		1024, 
		"tran_log/%08X%08X%08X", 
		ThisTimeLineID,	
		(uint32)(segmentNumber / TRAN_LOG_SEG_NUMBER),
	    (uint32)(segmentNumber % TRAN_LOG_SEG_NUMBER));
     
    if (*useExistent)
	{
        fileDescriptor = FileOpenBase(path, O_RDWR | PG_BINARY, S_IRUSR | S_IWUSR);
		if (fileDescriptor < 0)
			Log("error");
		else
			return fileDescriptor;
	}

	snprintf(tmppath, 1024, "tran_log/xlogtemp.%d", (int)getpid());

    unlink(tmppath);

	/* do not use get_sync_bit() here --- want to fsync only at end of fill */
	fileDescriptor = FileOpenBase(tmppath, 
		               O_RDWR | O_CREAT | O_EXCL | PG_BINARY,
					   S_IRUSR | S_IWUSR);
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

	if (fsync(fileDescriptor) != 0)
        Log("error");

	if (close(fileDescriptor))
        Log("error");

	{
        uint32* log = (uint32)(segmentNumber / TRAN_LOG_SEG_NUMBER);
		uint32* seg = (uint32)(segmentNumber % TRAN_LOG_SEG_NUMBER); 
		int     find_free = *useExistent; 
		int*    max_advance = TRAN_LOG_FILES_COUNT;
		int     use_lock = useLock;

		


	}
}

void TranLogWrite(TranLogWriteProgress writeRequest, bool flexible, bool tranLogSwitch)
{
	TranLogProgress = LogState->LogProgress;
	TranLogWriteState* Write = &LogState->WriteState;
	int current = Write->currentIndex;
	int ispartialpage;
	int	useExistentFile = 0;

	while (TranLogProgress.WritePosition.log_id < writeRequest.WritePosition.log_id || 
		       (TranLogProgress.WritePosition.log_id == writeRequest.WritePosition.log_id 
			 && TranLogProgress.WritePosition.tran_offset < writeRequest.WritePosition.tran_offset))
	{
		TranLogProgress.WritePosition = LogState->blocks[current];

		ispartialpage = writeRequest.WritePosition.log_id < TranLogProgress.WritePosition.log_id || 
		       (writeRequest.WritePosition.log_id == TranLogProgress.WritePosition.log_id 
			 && writeRequest.WritePosition.tran_offset < TranLogProgress.WritePosition.tran_offset);

	    if (!((TranLogProgress.WritePosition.log_id == openLogId &&
  	          (TranLogProgress.WritePosition.tran_offset - 1) / TRAN_LOG_SEG_SIZE == openLogSeg)))
		{
			openLogId = TranLogProgress.WritePosition.log_id;
            openLogSeg = (TranLogProgress.WritePosition.tran_offset - 1) / TRAN_LOG_SEG_SIZE;

			/* create/use new log file */
			useExistentFile = 1;
			openLogFile = TranLogIdFileInit(openLogSeg, &useExistentFile, 1);
		}
	}
}


void TranLogInsert(
	unsigned int rmid, 
	unsigned char info, 
	tran_log_data* tranData)
{
	int                  i;
	unsigned int         length;
	unsigned int         freeSpace;
	tran_log_data*       tranDataItem;
	unsigned int	     buffers[4];
	int		             buffers_in_use[4];
	tran_log_record      tranLogHeader;
	TranLogInsertState*  insertState = &LogState->InsertState;
	TranLogWriteProgress writeProgress;

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

	freeSpace = TRAN_LOG_BLOCK_SIZE - (insertState->currentPosition - (char*)insertState->currentPage);
    if (freeSpace < sizeof(tran_log_record))
	{
		int notExceeded;
		int nextIndex = (insertState->currentBlockIndex == LogState->highestBlock) ? 0 : insertState->currentBlockIndex + 1;
		tran_log_id oldId = insertState->blocks[nextIndex];
         
	    
		if (oldId >= TranLogProgress.WritePosition)
		{
			tran_log_id a = oldId;
			tran_log_id b = TranLogProgress.WritePosition;

			notExceeded =   (a.log_id < b.log_id 
						 || (a.log_id == b.log_id 
						  && a.tran_offset <= b.tran_offset));

			if (!notExceeded)
			{
				tran_log_id finishedPageId = insertState->blocks[insertState->currentBlockIndex];
	            
				volatile TranLogState* logStateLocal = LogState;
				SpinLockAcquire(logStateLocal->locker, NULL, 0);

				if (logStateLocal->LogProgress->WritePosition < finishedPageId)
					logStateLocal->LogProgress->WritePosition = finishedPageId;

				TranLogProgress = logStateLocal->LogProgress;

				SpinLockRelease(logStateLocal->locker);
			}

			if (oldId >= TranLogProgress.WritePosition)
		    {
				writeProgress.WritePosition = oldId;
				writeProgress.FlushPosition.log_id = 0;
				writeProgress.FlushPosition.tran_offset = 0;
			}
		}
	}
	
}
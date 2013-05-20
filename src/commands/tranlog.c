#include "tranlog.h"

static struct TranLogState* LogState = NULL; 
static struct TranLogProgress  = {{0, 0}, {0, 0}};


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
		int nextIndex = (insertState->currentBlockIndex == LogState->highestBlock) ? 0 : insertState->currentBlockIndex + 1;
		tran_log_id oldId = blocks[nextIndex];

        tran_log_id a = oldId;
        tran_log_id b = TranLogProgress.WritePosition;


		struct tran_log_id
{
	unsigned int		  log_id;	
	unsigned int		  tran_offset;
};


		#define XLByteLE(a, b)		\
			((a).xlogid < (b).xlogid || \
			 ((a).xlogid == (b).xlogid && (a).xrecoff <= (b).xrecoff))


	}
	
}
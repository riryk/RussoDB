#include "modify.h"


typedef enum BufAccessType
{
	BUF_ACCESS_NORMAL,					/* Normal random access */
	BUF_ACCESS_BULKREAD,				/* Large read-only scan  */
	BUF_ACCESS_BULKWRITE,				/* Large multi-block write */
	BUF_ACCESS_VACUUM					/* VACUUM */
} BufAccessType;

typedef struct BufAccessState
{
    BufAccessType   Type,
	int			    BufferArraySize;
    int			    Current;
	int             WasInRing;
	int		        Buffers[1];
} BufAccessState;

typedef struct RowType
{
} RowType;


typedef struct Attr1
{
	unsigned char		header;
	char		        data[1];		
} Attr1;

typedef struct Attr2
{
	char		length[4];		
	char		data[1];
} Attr2;


typedef struct HeapRowHeader
{
	unsigned short	Mask;
    unsigned short	Mask2;
    unsigned short  Offset;
	unsigned char	Bits[1];
} HeapRowHeader;

typedef struct MinimalRowHeader
{   
	unsigned short	Length;	
	unsigned short	Mask;
	unsigned short	Mask2;
} MinimalRowHeader;

typedef struct RowAttribute
{
	signed short	Length;
	char		    Align;
	int		        DimentionsCount;
	int			    TypeId;
	int		        TypeParam;
} RowAttribute;

typedef struct RowDescription
{
	int			  AttributesCount;
	RowAttribute* Attributes;
}RowDescription;


struct HeapTableRow
{
	int		          Length;
    BlockId           Block;
	long              PositionId;
	int		          TableId;		
	HeapRowHeader*    Header;
	MinimalRowHeader  MinimlRow;
    RowDescription    Description;
	unsigned int*     Values;
	unsigned int*     Nulls;
};

struct TableRow
{
	HeapTableRow*	  physicalRow;
	MinimalRowHeader* minimalRow;
    int		         shouldFree;  // this indicates that this structure has been 
	                             // allocated by malloc function and needs to be freed
	RowDescription   Description;
	unsigned int*     Values;
	unsigned int*     Nulls;
};

struct ModifyState
{
};

typedef struct RelationOptions
{
    int fillFactor;
}
RelationOptions;

struct Relation
{
    int              HasIds;
	Attr2            Options;
	StorageRelation  Storage;
	RelationFileInfo FileInfo;
};

typedef struct InsertState
{
	BufferAccessStrategy strategy;		/* our BULKWRITE strategy object */

	int	     currentBufId;	
}
InsertState;


static HashTable* RowTypeHash = NULL;

HeapTableRow* ClonePhysicalRow(HeapTableRow* row)
{
    HeapTableRow*  newRow;
	int newSize = (sizeof(HeapTableRow) + 7) & ~7);

	if (row->Header == NULL)
		return NULL;

	newRow = (HeapTableRow*)malloc(newSize + row->Length);
	newRow->Length = row->Length;
	newRow->Block = row->Block;
	newRow->PositionId = row->PositionId;

	newRow->TableId = row->TableId;
	newRow->Header = (HeapRowHeader*)((char*)newRow + newSize);

	memcpy((char*)newRow->Header, (char*)row->Header, row->Length);
	return newRow;
}

HeapTableRow* CopyFromMinimalRow(MinimalRowHeader* minRow)
{
    struct HeapTableRow*  newRow;
    int alignmentOffset = ((offsetof(HeapRowHeader, Mask2) - sizeof(unsigned int)) / 8 * 8);
	int newSize = (sizeof(HeapTableRow) + 7) & ~7);

	newRow = (struct HeapTableRow*)malloc(newSize + minRow->Length + alignmentOffset);
	newRow->Length = minRow->Length;
	newRow->Block.high = 1 << 32 - 1;

	newRow->PositionId = 0;
    newRow->TableId = 0;

	newRow->Header = (HeapRowHeader*)((char*)newRow + newSize);

	memcpy((char*)newRow->Header + alignmentOffset, minRow, minRow->Length);
	memset(newRow->Header, 0, offsetof(HeapRowHeader, Mask2));
	return result;
}

RowType* LookupRowType(unsigned int typeId, int flags)
{
	struct HashTableSettings*		hashTableSett;

    if (RowTypeHash == NULL)
	{
		MemSet(&hashTableSett, 0, sizeof(hashTableSett));

		hashTableSett->KeyLength = sizeof(unsigned int);
	    hashTableSett->ValueLength = sizeof(RowType);
		hashTableSett->HashFunc = HashSimple;
	}
}

void GetRowTypeInfo(
	unsigned int typeId, 
	int typeMode)
{
    if (typeId != TYPE_REC_OID)
	{
        RowType* type;
	}
	else
	{

	}
}

unsigned int* SimplifyAttribute(
	unsigned int value,
    unsigned int typeId, 
	int typeMode)
{
	int			    AttributesCount;
	RowAttribute*   Attributes;
	HeapRowHeader*  OldData;

    RowDescription* rowDesc = lookup_rowtype_tupdesc_noerror(typeId, typeMod, true);
	if (rowDesc == NULL)
		return value;	

    Attributes = rowDesc->Attributes;
	AttributesCount = rowDesc->AttributesCount;

	OldData = (HeapRowHeader*)(Attr2*)(char*)value;
}

HeapTableRow* CopyFromValueaArray(
	RowDescription    descriptor,
	unsigned int*     values,
	unsigned int*     nulls)
{
    int			  numberOfAttributes = descriptor.AttributesCount;;
	RowAttribute* attributes = descriptor->Attributes;
	int           i;
	int           hasNulls = 0;

	if (numberOfAttributes > MAX_ATTRIBUTES_COUNT)
		Error("Exceeded max attributes count: %d", MAX_ATTRIBUTES_COUNT);

	for (i = 0; i < numberOfAttributes; i++)
	{
		if (nulls[i])
			hasNulls = 1;
        else if (attributes[i]->Length == -1 &&
				 attributes[i]->Align == 'd' &&
				 attributes[i]->DimentionsCount == 0 && 
                 ((Attr1*)((char*)values[i]))->va_header & 0x03 != 0x00)
		{
            values[i] = SimplifyAttribute(
	                     values[i],
			             attributes[i].TypeId,
	                     attributes[i].TypeParam);
		}
	}
}

HeapTableRow* CopySlotTuple(TableRow* row)
{
	if (row->physicalRow != NULL)
		return ClonePhysicalRow(row->physicalRow); 

	if (slot->minimalRow != NULL)
		return CopyFromMinimalRow(row->minimalRow);

	return CopyFromValueaArray(
		row->Description,
	    row->Values,
		row->Nulls);
}

HeapTableRow* ConvertFromVirtualRowToPhysicalRow(TableRow* row)
{
    if (row->physicalRow && row->shouldFree)
      return row->physicalRow;

	row->physicalRow = CopySlotTuple(row); 
    row->shouldFree = 1;
	return row->physicalRow;
}

static HeapTableRow* BeforeInsert(
	Relation* relation, 
	HeapTableRow* row, 
	unsigned int tranId,
	unsigned int commandId, 
	int options)
{
	row->Header->Mask &= ~(TRAN_MASK);
    row->Header->Mask2 &= ~(ATTR_MASK);
    row->Header->Mask |= TRAN_MAX_INVALID;

    return row;
}

unsigned int GetCurrentTranId()
{
    return 0;
}

int GetBlockNumber(int buffer)
{
    if (buffer < 0)
		return LocalBufferDescriptions[-buffer - 1].blockNumber;

    return BufferDescriptions[buffer - 1].blockNumber;
}

int  GetBufferIdForTableRow(
		   Relation* relation,
		   int length,
		   int otherBuffer, 
		   int options,
		   BufAccessState* state,
		   int* vmBuffer, 
		   int* vmBufferOther)
{
	int		     useFreeSpaceManager = !(options & TABLE_INSERT_SKIP_FREE_SPACE_MANAGER);
	int          relationFillFactor;
    unsigned int freeSpace; 
	unsigned int otherBlock;
    unsigned int returnBlock;

    if (length > MaxHeapTupleSize)
		Error("Exceeded max length");
    
	relationFillFactor = relation->Options
	    ? ((RelationOptions)relation->Options)->fillFactor 
		: DEFAULT_FILLFACTOR;
    
    freeSpace = BLOCK_SIZE * (100 - relationFillFactor) / 100;

    if (otherBuffer != 0)
	    otherBlock = GetBlockNumber(otherBuffer);
	else
		otherBlock = InvalidBlockNumber;	

	if (length + freeSpace > MaxHeapTupleSize)
	{
		returnBlock = 1 << 32 - 1;
		useFreeSpaceManager = 0;
	}
	else if (state && state->Current != 1 << 32 - 1)
		returnBlock = GetBlockNumber(state->Current);
	else
		returnBlock = relation->Storage != NULL ? 
		              relation->Storage->CurrentBlock
					  : 1 << 32 - 1;

	if (returnBlock == 1 << 32 - 1 && useFreeSpaceManager)
	    return 0;

	return 0;
}

void PutRowIntoBlock(
	struct Relation relation,
	int buffer,
	HeapTableRow* row)
{
	int pageHeaderSize;
	int newRowPosition;
    int alignedSize;

	int newStart;
    int newEnd;

	char* pageHeaderChar;
    PageHeader* pageHeader;
	PageItemData* itemData;
    
	if (buffer < 0)
        pageHeaderChar = (char*)LocalBuffers[-buffer-1];
	else
        pageHeaderChar = (char*)(BufferBlocks + (buffer - 1) * BLOCK_SIZE);
 
    pageHeader = (PageHeader*)pageHeaderChar;	
    pageHeaderSize = offsetof(PageHeader, rowInfoData);

	newRowPosition = pageHeader->startFreeSpace <= pageHeaderSize ?
		0 : (pageHeader->startFreeSpace - pageHeaderSize) / sizeof(PageItemData);
    
    newRowPosition++;

	newStart = pageHeader->startFreeSpace + sizeof(PageItemData);
	alignedSize = (row->Length + 7) & ~7);

	newEnd = pageHeader->endFreeSpace - alignedSize;

    if (newStart > newEnd)
		return 1 << 32 - 1;

	itemData = (PageItemData)(&page->rowInfoData[newRowPosition - 1]);

	itemData->offsetToRow = newEnd;
	itemData->state = ROW_UNUSED;
	itemData->length = row->Length;

	memcpy((char*)pageHeaderChar + newEnd, row->Header, alignedSize);

	pageHeader->startFreeSpace = newStart;
	pageHeader->endFreeSpace = newEnd;
}

static TableRow* Insert(
		   Relation* relation,
		   TableRow* row,
		   TableRow* planRow,
		   ModifyState* mState,
		   int tranId,
		   int commandId,
		   int canSet,
		   int options)
{
    unsigned int tranId = GetCurrentTranId();
	int bufferId;
	int addBuffer;
    void* page;

	tran_log_insert logInsRec;
	tran_log_header logHeader;
	tran_log_data tranData[3];

	HeapTableRow* heapRow = BeforeInsert(relation, row, tranId, commandId, options);

	bufferId = GetBufferIdForTableRow(
		   relation,
		   heapRow->Length,
		   0, 
		   options,
		   mState,
		   &addBuffer, 
		   NULL);

    PutRowIntoBlock(relation, bufferId, heapRow);

	if (buffer < 0)
        (&LocalBufferDescriptions[-buffer - 1])->flags |= BUFFER_DIRTY;
	else
	    (&BufferDescriptions[buffer - 1])->flags |= BUFFER_DIRTY;

	page = GetBlockNumber(bufferId);

	logInsRec.allCleared = 1;

	logInsRec.tableSpaceId = relation->FileInfo->tableSpaceId;		
	logInsRec.databaseId = relation->FileInfo->databaseId;		
	logInsRec.relationId = relation->FileInfo->relationId;		

    tranData[0].data = (char*)&logInsRec;
    tranData[0].length = offsetof(tran_log_insert, allCleared) + sizeof(unsigned short));
    tranData[0].buffer = 0;
    tranData[0].next = &(tranData[1]);

	logHeader.mask2 = heapRow->Header->Mask2;
	logHeader.mask = heapRow->Header->Mask;
	logHeader.offset = heapRow->Header->Offset;

	tranData[1].data = (char*)&logHeader;
    tranData[1].length = (offsetof(tran_log_header, offset) + sizeof(unsigned char));
	tranData[1].buffer = bufferId;
	tranData[1].bufferStandard = 1;
	tranData[1].next = &(tranData[2]);

	row->physicalRow->Header

	tranData[2].data = (char*)row->physicalRow->Header + offsetof(HeapRowHeader, Bits);
	tranData[2].length = row->physicalRow->Length - offsetof(HeapRowHeader, Bits);
    tranData[2].buffer = bufferId;
	tranData[2].bufferStandard = 1;
	tranData[2].next = NULL;

}
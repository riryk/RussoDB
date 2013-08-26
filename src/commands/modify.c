#include "modify.h"
#include <stddef.h>     /* offsetof */
#include "buffermanager.h"
#include "relation_storage.h"

typedef enum BufAccessType
{
	BUF_ACCESS_NORMAL,					/* Normal random access */
	BUF_ACCESS_BULKREAD,				/* Large read-only scan  */
	BUF_ACCESS_BULKWRITE,				/* Large multi-block write */
	BUF_ACCESS_VACUUM					/* VACUUM */
} BufAccessType;

typedef struct BufAccessState
{
    BufAccessType   Type;
	int			    BufferArraySize;
    int			    Current;
	int             WasInRing;
	int		        Buffers[1];
} BufAccessState;

typedef struct RowType
{
	int A;
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

struct MinimalRowHeader
{   
	unsigned short	Length;	
	unsigned short	Mask;
	unsigned short	Mask2;
};

struct RowAttribute
{
	signed short	Length;
	char		    Align;
	int		        DimentionsCount;
	unsigned int    TypeId;
	int		        TypeParam;
};

typedef struct RowDescription
{
	int			  AttributesCount;
	struct RowAttribute* Attributes;
}RowDescription;


struct HeapTableRow
{
	int		          Length;
    struct BlockId           Block;
	long              PositionId;
	int		          TableId;		
	struct HeapRowHeader*    Header;
	struct MinimalRowHeader  MinimlRow;
    struct RowDescription    Description;
	unsigned int*     Values;
	unsigned int*     Nulls;
};

struct TableRow
{
	struct HeapTableRow*	  physicalRow;
	struct MinimalRowHeader* minimalRow;
    int		         shouldFree;  // this indicates that this structure has been 
	                             // allocated by malloc function and needs to be freed
	struct RowDescription   Description;
	unsigned int*     Values;
	unsigned int*     Nulls;
};

struct ModifyState
{
	int A;
};

typedef struct RelationOptions
{
    int fillFactor;
}
RelationOptions;

struct Relation
{
    int              HasIds;
	struct Attr2            Options;
	struct  StorageRelation  Storage;
	struct RelationFileInfo FileInfo;
};

typedef struct InsertState
{
	//BufferAccessStrategy strategy;		/* our BULKWRITE strategy object */

	int	     currentBufId;	
}
InsertState;


static struct HashTable* RowTypeHash = NULL;

struct HeapTableRow* ClonePhysicalRow(struct HeapTableRow* row)
{
    struct HeapTableRow*  newRow;
	int newSize = ((sizeof(struct HeapTableRow) + 7) & ~7);

	if (row->Header == NULL)
		return NULL;

	newRow = (struct HeapTableRow*)malloc(newSize + row->Length);
	newRow->Length = row->Length;
	newRow->Block = row->Block;
	newRow->PositionId = row->PositionId;

	newRow->TableId = row->TableId;
	newRow->Header = (struct HeapRowHeader*)((char*)newRow + newSize);

	memcpy((char*)newRow->Header, (char*)row->Header, row->Length);
	return newRow;
}

struct HeapTableRow* CopyFromMinimalRow(struct MinimalRowHeader* minRow)
{
    struct HeapTableRow*  newRow;  
    int alignmentOffset = ((offsetof(struct HeapRowHeader, Mask2) - sizeof(unsigned int)) / 8 * 8);
	int newSize = ((sizeof(struct HeapTableRow) + 7) & ~7);

	newRow = (struct HeapTableRow*)malloc(newSize + minRow->Length + alignmentOffset);
	newRow->Length = minRow->Length;
	//newRow->Block.high = 1 << 32 - 1;

	newRow->PositionId = 0;
    newRow->TableId = 0;

	newRow->Header = (struct HeapRowHeader*)((char*)newRow + newSize);

	memcpy((char*)newRow->Header + alignmentOffset, minRow, minRow->Length);
	memset(newRow->Header, 0, offsetof(struct HeapRowHeader, Mask2));
	return newRow;
}

struct RowType* LookupRowType(unsigned int typeId, int flags)
{
	struct HashTableSettings*		hashTableSett;

    if (RowTypeHash == NULL)
	{
		memset(&hashTableSett, 0, sizeof(hashTableSett));

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
	struct RowAttribute*   Attributes;
	struct HeapRowHeader*  OldData;

    struct RowDescription* rowDesc;// = lookup_rowtype_tupdesc_noerror(typeId, typeMode, 1);
	if (rowDesc == NULL)
		return value;	

    Attributes = rowDesc->Attributes;
	AttributesCount = rowDesc->AttributesCount;

	OldData = (struct HeapRowHeader*)(struct Attr2*)(char*)value;
}

struct HeapTableRow* CopyFromValueaArray(
	struct RowDescription    descriptor,
	unsigned int*     values,
	unsigned int*     nulls)
{
    int			  numberOfAttributes = descriptor.AttributesCount;
	struct RowAttribute* attributes = descriptor.Attributes;
	int           i;
	int           hasNulls = 0;

	if (numberOfAttributes > MAX_ATTRIBUTES_COUNT)
		;//Error("Exceeded max attributes count: %d", MAX_ATTRIBUTES_COUNT);

	for (i = 0; i < numberOfAttributes; i++)
	{
		if (nulls[i])
			hasNulls = 1;
        else if (attributes[i].Length == -1 &&
				 attributes[i].Align == 'd' &&
				 attributes[i].DimentionsCount == 0 && 
                 ((struct Attr1*)((char*)values[i]))->header & 0x03 != 0x00)
		{
            /*values[i] = SimplifyAttribute(
	                     values[i],
			             attributes[i].TypeId,
	                     attributes[i].TypeParam);*/
		}
	}
}

struct HeapTableRow* CopySlotTuple(struct TableRow* row)
{
	if (row->physicalRow != NULL)
		return NULL;
		//return ClonePhysicalRow(row->physicalRow); 

	if (row->minimalRow != NULL)
		return NULL;
		//return CopyFromMinimalRow(row->minimalRow);

	return NULL;
	/*return CopyFromValueaArray(
		row->Description,
	    row->Values,
		row->Nulls);*/
}

struct HeapTableRow* ConvertFromVirtualRowToPhysicalRow(struct TableRow* row)
{
    if (row->physicalRow && row->shouldFree)
      return row->physicalRow;

	row->physicalRow = CopySlotTuple(row); 
    row->shouldFree = 1;
	return row->physicalRow;
}

//static HeapTableRow* BeforeInsert(
//	struct Relation* relation, 
//	struct HeapTableRow* row, 
//	unsigned int tranId,
//	unsigned int commandId, 
//	int options)
//{
//	row->Header->Mask &= ~(TRAN_MASK);
//    row->Header->Mask2 &= ~(ATTR_MASK);
//    row->Header->Mask |= TRAN_MAX_INVALID;
//
//    return row;
//}

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
		   struct Relation* relation,
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
		;//Error("Exceeded max length");
    
	/*relationFillFactor = relation->Options
	    ? ((RelationOptions)relation->Options)->fillFactor 
		: DEFAULT_FILLFACTOR;*/
    
    freeSpace = BLOCK_SIZE * (100 - relationFillFactor) / 100;

    if (otherBuffer != 0)
	    otherBlock = GetBlockNumber(otherBuffer);
	else
		otherBlock = 1; //InvalidBlockNumber;	

	if (length + freeSpace > MaxHeapTupleSize)
	{
		returnBlock = 1 << 32 - 1;
		useFreeSpaceManager = 0;
	}
	else if (state && state->Current != 1 << 32 - 1)
		returnBlock = GetBlockNumber(state->Current);
	else
		/*returnBlock = relation->Storage != NULL ? 
		              relation->Storage.CurrentBlock
					  : 1 << 32 - 1;*/

	if (returnBlock == 1 << 32 - 1 && useFreeSpaceManager)
	    return 0;

	return 0;
}

void PutRowIntoBlock(
	struct Relation relation,
	int buffer,
	struct HeapTableRow* row)
{
	int pageHeaderSize;
	int newRowPosition;
    int alignedSize;

	int newStart;
    int newEnd;

	char* pageHeaderChar;
    struct PageHeader* pageHeader;
	struct PageItemData* itemData;
    
	if (buffer < 0)
        pageHeaderChar = (char*)LocalBuffers[-buffer-1];
	else
        pageHeaderChar = (char*)(BufferBlocks + (buffer - 1) * BLOCK_SIZE);
 
    pageHeader = (struct PageHeader*)pageHeaderChar;	
    pageHeaderSize = offsetof(struct PageHeader, rowInfoData);

	newRowPosition = pageHeader->startFreeSpace <= pageHeaderSize ?
		0 : (pageHeader->startFreeSpace - pageHeaderSize) / sizeof(struct PageItemData);
    
    newRowPosition++;

	newStart = pageHeader->startFreeSpace + sizeof(struct PageItemData);
	alignedSize = ((row->Length + 7) & ~7);

	newEnd = pageHeader->endFreeSpace - alignedSize;

    if (newStart > newEnd)
		return 1 << 32 - 1;

	itemData = (struct PageItemData*)(&pageHeader->rowInfoData[newRowPosition - 1]);

	itemData->offsetToRow = newEnd;
	itemData->state = ROW_UNUSED;
	itemData->length = row->Length;

	memcpy((char*)pageHeaderChar + newEnd, row->Header, alignedSize);

	pageHeader->startFreeSpace = newStart;
	pageHeader->endFreeSpace = newEnd;
}

static struct TableRow* Insert(
		   struct Relation* relation,
		   struct TableRow* row,
		   struct TableRow* planRow,
		   struct ModifyState* mState,
		   int tranId,
		   int commandId,
		   int canSet,
		   int options)
{
    unsigned int tranIdCurrent = GetCurrentTranId();
	int bufferId;
	int addBuffer;
    void* page;

	struct tran_log_insert logInsRec;
	struct tran_log_header logHeader;
	struct tran_log_data tranData[3];

	struct HeapTableRow* heapRow = BeforeInsert(relation, row, tranId, commandId, options);

	bufferId = GetBufferIdForTableRow(
		   relation,
		   heapRow->Length,
		   0, 
		   options,
		   mState,
		   &addBuffer, 
		   NULL);

    PutRowIntoBlock(*relation, bufferId, heapRow);

	if (bufferId < 0)
        (&LocalBufferDescriptions[-bufferId - 1])->flags |= BUFFER_DIRTY;
	else
	    (&BufferDescriptions[bufferId - 1])->flags |= BUFFER_DIRTY;

	page = GetBlockNumber(bufferId);

	logInsRec.allCleared = 1;

	logInsRec.tableSpaceId = relation->FileInfo.tableSpaceId;		
	logInsRec.databaseId = relation->FileInfo.databaseId;		
	logInsRec.relationId = relation->FileInfo.relationId;		

    tranData[0].data = (char*)&logInsRec;
    tranData[0].length = (offsetof(struct tran_log_insert, allCleared) + sizeof(unsigned short));
	tranData[0].bufferId = 0;
    tranData[0].next = &(tranData[1]);

	logHeader.mask2 = heapRow->Header->Mask2;
	logHeader.mask = heapRow->Header->Mask;
	logHeader.offset = heapRow->Header->Offset;

	tranData[1].data = (char*)&logHeader;
    tranData[1].length = (offsetof(struct tran_log_header, offset) + sizeof(unsigned char));
	tranData[1].bufferId = bufferId;
	tranData[1].bufferStandard = 1;
	tranData[1].next = &(tranData[2]);

	//row->physicalRow->Header

	tranData[2].data = (char*)row->physicalRow->Header + offsetof(HeapRowHeader, Bits);
	tranData[2].length = row->physicalRow->Length - offsetof(HeapRowHeader, Bits);
    tranData[2].bufferId = bufferId;
	tranData[2].bufferStandard = 1;
	tranData[2].next = NULL;

}
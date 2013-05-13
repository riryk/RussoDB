#include "insert.h"

typedef struct RowType
{
} RowType;

typedef struct BlockId
{
	unsigned short		high;
	unsigned short		low;
} BlockId;

typedef struct
{
	unsigned char		header;
	char		        data[1];		
} Attr1;

struct struct
{
	char		length[4];		
	char		data[1];
} Attr2;

typedef struct HeapRowHeader
{
	unsigned short	Mask;
    unsigned short	Mask2;
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
} RowAttribute;

typedef struct RowDescription
{
	int			  AttributesCount;
	RowAttribute* Attributes;
}RowDescription;

typedef struct HeapTableRow
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
} HeapTableRow;

typedef struct TableRow
{
	HeapTableRow	physicalRow;
    int		        shouldFree;  // this indicates that this structure has been 
	                             // allocated by malloc function and needs to be freed
} TableRow;

typedef struct ModifyState
{
} ModifyState;

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
    HeapTableRow*  newRow;
    int alignmentOffset = ((offsetof(HeapRowHeader, Mask2) - sizeof(unsigned int)) / 8 * 8);
	int newSize = (sizeof(HeapTableRow) + 7) & ~7);

	newRow = (HeapTableRow)malloc(newSize + minRow->Length + alignmentOffset);
	newRow->Length = minRow->Length;
	newRow->Block = 1 << 32 - 1;
	newRow->PositionId = 0;
    newRow->TableId = 0;

	newRow->Header = (HeapRowHeader*)((char*)newRow + newSize);

	memcpy((char*)newRow->Header + alignmentOffset, minRow, minRow->Length);
	memset(newRow->Header, 0, offsetof(HeapRowHeader, Mask2));
	return result;
}

RowType* LookupRowType(unsigned int typeId, int flags)
{
    if (RowTypeHash == NULL)
	{
		HashTableSettings		hashTableSett;

		MemSet(&hashTableSett, 0, sizeof(hashTableSett));


		hashTableSett.KeyLength = sizeof(unsigned int);
	    hashTableSett.ValueLength = sizeof(RowType);
		hashTableSett.HashFunc = HashSimple;
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

	OldData = (HeapRowHeader*)(Attr2)(char*)value;


}

HeapTableRow* CopyFromValueaArray(
	RowDescription    descriptor,
	unsigned int*     values,
	unsigned int*     nulls)
{
    int			  numberOfAttributes = descriptor->AttributesCount;;
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

		}
	}
}


HeapTableRow* ConvertFromVirtualRowToPhysicalRow(TableRow* row)
{
    if (row->physicalRow && row->shouldFree)
      return row->physicalRow;
  

}

static TableRow* Insert(
		   TableRow* row,
		   TableRow* planRow,
		   ModifyState* mState,
		   int canSet)
{

}
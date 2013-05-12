#include "insert.h"

typedef struct BlockId
{
	unsigned short		high;
	unsigned short		low;
} BlockId;

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
	int*              Values;
	int*              Nulls;
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

HeapTableRow* CopyFrom 

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
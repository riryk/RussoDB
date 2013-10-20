
#include "relrowmanager.h"
#include "stddef.h"
#include "types.h"

//const SIRelRowManager sRelRowManager = { createRelRow };
//const IRelRowManager relRowManager = &sRelRowManager;
//
int att_align_nominal(int len, char align)
{
    switch (align)// == 'i')
    {
    case 'i':
       return ALIGN_TYPE(ALIGN_INT, len);
    case 'c':
	   return (int)len;
    case 'd':
	   return ALIGN_TYPE(ALIGN_DOUBLE, len);
    case 's':
       return ALIGN_TYPE(ALIGN_SHORT, len);
    } 
	return -1;
}

void store_att_byval(char* data, uint newdatum, int16 attlen)
{
    switch (attlen)
    {
		case sizeof(char): 
			*(char*)(data) = (char)(newdatum & 0x000000ff);
			break;
	    case sizeof(int16):
			*(int16*)(data) = (int16)(newdatum & 0x0000ffff);
			break; 
		case sizeof(int): 
			*(int*)(data) = (int)(newdatum & 0xffffffff);
			break; 
		default:
			break; 
	}
}

size_t computeRowSize(int             relAttrsCount,
					  uint*           values,
					  Bool*           isnull)
{
	int      i;
	size_t   rowLen;

    for (i = 0; i < relAttrsCount; i++)
	{
		uint	 val;
		char*    valP;
		Bool     isPackable;

        if (isnull[i])
		    continue; 

		val = values[i];
		valP = (char*)val;
	}
}

/* Suppose that for each column value we have 4 byte alignment.
 * In this case, for example, if we have a column of char(1) data type.
 * sizeof(char(1)) = 1 byte. So we have to waste 3 additional bytes.
 */
RelRow createRelRow(void*           self,
					int             relAttrsCount,
					RelAttribute    relAttrs,
					Bool		    hasId,
				    uint*           values,
				    Bool*           isnull)
{
	IHashtableManager _ = (IHashtableManager)self;
	RelRow            row;
	RelRowHeader      rowHd;
	size_t            len;
	Bool              hasnull;
	int               i;
    uint8*            bit;                  
    uint8*            bitPrev;
	int			      bitmask;
	char*             data;

    for (i = 0; i < relAttrsCount; i++)
	{
		if (isnull[i])
			hasnull = True;
	}

	len = offsetof(SRelRowHeader, nullBits);
	if (hasnull)
		len += BITMAP_LEN(relAttrsCount);

	if (hasId)
		len += sizeof(uint);

	len = ALIGN(len);

	for (i = 0; i < relAttrsCount; i++)
	{
		uint	 val;
		char*    valP;
		Bool     isPackable;

        if (isnull[i])
		    continue;

		val = values[i];
		valP = (char*)val;

        if (ATT_CAN_PACK(&(relAttrs[i])) && CAN_MAKE_SHORT(valP))
		{
			len += SHORT_SIZE(valP);
		}
		else
		{
			len = att_align_nominal(len, relAttrs[i].align);

			if (relAttrs[i].len > 0)
				len += relAttrs[i].len;
			else if (relAttrs[i].len == -1)
				len += (IS_1B_E(valP) ? VARSIZE_1B_E(valP) :
				       (IS_1B(valP) ? VARSIZE_1B(valP) : VARSIZE_4B(valP)));
			else if (relAttrs[i].len == -1)
				len += strlen((char*)valP) + 1;
	    }
	}

	row = (RelRow)_->memManager->alloc(ALIGN(sizeof(SRelRow)) + len);
	row->data = rowHd = (RelRowHeader)((char*)row + ALIGN(sizeof(SRelRow)));
	row->len = len;

	BlockIdSet(&(row->self.block), (uint)0xFFFFFFFF);
	row->tblId = (uint)0;

	((Col_4b)rowHd)->col_4byte.header = (uint)len << 2;
	rowHd->mask2 = (rowHd->mask2 & ~NATTS_MASK) | relAttrsCount;
	rowHd->offset = len;

	data = (char*)rowHd + rowHd->offset;
	bit = hasnull ? rowHd->nullBits : NULL;
	if (bit != NULL)
	{
		bitPrev = &bit[-1];
		bitmask = 0x80;
	}
	else
	{
		bitPrev = NULL;
		bitmask = 0;
	}

    rowHd->mask &= ~(ROW_HASNULL | ROW_HASVARWIDTH | ROW_HASEXTERNAL);   

	for (i = 0; i < relAttrsCount; i++)
	{
       size_t   dataLen;

	   if (bit != NULL)
	   {
           if (bitmask != 0x80)
		   	   bitmask <<= 1;
		   else
		   {
			   bitPrev += 1;
               *bitPrev = 0x0;
		       bitmask = 1;
		   }

		   if (isnull[i])
		   {
               rowHd->mask |= ROW_HASNULL;
               continue;
		   }

           *bitPrev |= bitmask;
	   }

       if (relAttrs[i].byVal)
	   {
		   data = (char*)att_align_nominal(data, relAttrs[i].align);
		   store_att_byval(data, values[i], relAttrs[i].len);
		   dataLen = relAttrs[i].align;
	   }
	   else if (relAttrs[i].len == -1)
	   {
           char* pointer = (char*)values[i];
		   rowHd->mask |= ROW_HASVARWIDTH;
        
           if (IS_1B_E(pointer))
		   {
			   rowHd->mask |= ROW_HASEXTERNAL;
			   dataLen = VARSIZE_1B_E(pointer);
			   memcpy(data, pointer, dataLen);
		   }
		   else if (IS_1B(pointer))
		   {
			   dataLen = VARSIZE_1B(pointer);
			   memcpy(data, pointer, dataLen);   
		   }
		   else if ((relAttrs[i].storageStrategy != 'o') && CAN_MAKE_SHORT(pointer))
		   {
               dataLen = SHORT_SIZE(pointer);
			   SET_VARSIZE_1B(pointer,dataLen);
               memcpy(data + 1, VARDATA_4B(pointer), dataLen - 1);
		   }
		   else
		   {
			   data = (char*)att_align_nominal(data, relAttrs[i].align);
			   dataLen = VARSIZE_4B(pointer);
			   memcpy(data, pointer, dataLen);
		   }
	   }
	   else if (relAttrs[i].len == -2)
	   {
		   rowHd->mask |= ROW_HASVARWIDTH;
		   dataLen = strlen((char*)values[i]) + 1;
		   memcpy(data, (char*)values[i], dataLen);
	   }
	   else
	   {
		   data = (char*)att_align_nominal(data, relAttrs[i].align);
		   dataLen = relAttrs[i].len;
		   memcpy(data, (char*)values[i], dataLen);
	   }

	   data += dataLen;
	}
}

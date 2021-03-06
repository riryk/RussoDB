
#include "relrowmanager.h"
#include "stddef.h"
#include "types.h"
#include "block.h"

const SIRelRowManager sRelRowManager = 
{ 
	createRelRow,
    shortenRow
};

const IRelRowManager relRowManager = &sRelRowManager;

size_t computeRowSize(RelAttribute    relAttrs,
					  int             relAttrsCount,
					  uint*           values,
					  Bool*           isnull)
{
	int      i;
	size_t   rowLen = 0;

    for (i = 0; i < relAttrsCount; i++)
	{
		uint	      val;
		Bool          isPackable;
		SRelAttribute att;

        if (isnull != NULL && isnull[i])
		    continue; 

		val = values[i];
		att = relAttrs[i];

		if (ATT_CAN_PACK(&att) && CAN_COMPRESS_TO_SHORT((char*)val))
		{
		    rowLen += LONG_VAR_LEN_ATTR_SIZE((char*)val);
		    continue;
		}
		
		/* Attribute len -1 indicates that the length of attribute is 
		 * the same as the size of the attribute.  
		 * Add alignment changes to rowLen.
		 */
		if (!(att.len == -1 && IS_FIRST_BIT_0((char*)val)))
			rowLen = ATT_ALIGN(rowLen, att.align);

		/* If we have an exact attribute length, we simply
		 * increate rowLen for it. */
        if (att.len > 0)
		{
			rowLen += att.len;
			continue;
		}

        if (att.len == -1)
		{
			rowLen += ComputeSize(val);
			continue;
		}

		rowLen += strlen((char*)val) + 1;
	}

	return rowLen;
}

void buildRelRow(RelAttribute    relAttrs,
				 int             relAttrsCount,
                 uint*           values,
				 Bool*           isnull,
                 char*           dataP,
                 size_t          dataLen,
				 uint16*         mask,
                 uint8*          nullBits)
{
	int        i;
	uint8*     nullBitPrev;
    int        nullBitMask;
	char*      valP;
	size_t     step;

	/* Clear the first 3 bits from mask. */
    *mask &= ~(ROW_HASNULL | ROW_HASVARWIDTH | ROW_HASEXTERNAL);       

	if (nullBits != NULL)
	{
       nullBitPrev = &nullBits[-1];
	   nullBitMask = BIT_8_SET; 
	}

	for (i = 0; i < relAttrsCount; i++)
	{
       /* bit != NULL means that there is at least one 
	    * attribute with null value. 
		* We have an isnull boolean array which indicates what 
		* attributes are null.
		* For example: isnull = [0,0,0,1, 1,1,0,0, 0,1,0,1, 0,1,0,0, 1,0,1,0], 
		* nullBitPrev = [???? ???? ???? ???? ????], nullBitMask = 1000 0000
		* The first step:  [0000 0000 ???? ???? ????], 0000 0001
		*                  [0000 0001 ???? ???? ????]
		* The second step:  0000 0010, [0000 0011 ???? ???? ????]
		* The third step:   0000 0100, [0000 0111 ???? ???? ????]
		* The fourth step:  0000 1000, [0000 0111 ???? ???? ????]
		* The fifth step:   0001 0000, [0000 0111 ???? ???? ????]
		* The sixth step:   0010 0000, [0000 0111 ???? ???? ????]
		* The seventh step: 0100 0000, [0100 0111 ???? ???? ????]
		* The eightth step: 1000 0000, [1100 0111 ???? ???? ????]
		* The ninth step:   [1100 0111 0000 0000 ????], 0000 0001
		*                   [1100 0111 0000 0001 ????]
		*/ 
	   if (nullBits != NULL)
	   {
           if (nullBitMask != BIT_8_SET)
		   	   nullBitMask <<= 1;
		   else
		   {
			   /* When we are here this means that we have filled 
			    * a nullBits element and we need to go to the next element. */
			   nullBitPrev += 1;
               *nullBitPrev = 0x0;
		       nullBitMask = 1;
		   }

		   if (isnull[i])
		   {
               *mask |= ROW_HASNULL;
               continue;
		   }

           *nullBitPrev |= nullBitMask;
	   }

	   if (relAttrs[i].byVal)
	   {
		   /* dataP is allocated by malloc. By default it will be int aligned. */
		   dataP = (char*)ATT_ALIGN(dataP, relAttrs[i].align);
		   step = relAttrs[i].len;
		   SET_ATTR_VALUE(dataP, values[i], step);
		   dataP += step;
		   continue;
	   }

	   /* len = -2 means that we are dealinf with a null terminated 
	    * C string */
	   if (relAttrs[i].len == -2)
	   {
           *mask |= ROW_HASVARWIDTH;
		   step = strlen((char*)values[i]) + 1;
		   memcpy(dataP, (char*)values[i], step);
           dataP += step;
		   continue;
	   }

	   if (relAttrs[i].len != -1)
	   {
		   dataP = (char*)ATT_ALIGN(dataP, relAttrs[i].align);;
		   step = relAttrs[i].len;
		   memcpy(dataP, (char*)values[i], step);
           dataP += step;
		   continue;
	   }

	   /* If we are here, that means that relAttrs[i].len = -1. */
       valP = (char*)values[i];
	   *mask |= ROW_HASVARWIDTH;

	   /* We have got attr.len = -1 which means a variable-length attribute.
	    * 1 byte max value is 1111 1111 = 255. 
		* The first byte of values[i] is a code
		* The second byte is the length of the attribute.
		* 254 bytes are left for the data. 
	    */
	   if (IS_FIRST_BYTE_1(valP))
	   {
		   *mask |= ROW_HASEXTERNAL;
		   step = GET_SECOND_BYTE(valP);
		   memcpy(dataP, valP, step);
           dataP += step;
		   continue;
	   }
       
	   /* Let's take the first byte: 0110 0011 
	    * If the first bit is 1 we are dealing with a short 
		* variable-length attribute. Short-variable length can 
		* take 7 bits. The maximum number is 0111 1111 = 127.
		* By applying this small trick we economize 8 bit space for
		* every attribute.
	    */
	   if (IS_FIRST_BIT_1(valP))
	   {
		   step = CUT_THE_LAST_BIT_AND_TAKE_7_BITS(valP);
		   memcpy(dataP, valP, step);   
           dataP += step;
		   continue;
	   }

	   if ((relAttrs[i].storageStrategy != 'o') 
		   && CAN_COMPRESS_TO_SHORT(valP))
	   {
           step = LONG_VAR_LEN_ATTR_SIZE(valP);
		   MARK_AS_SHORT_VAR_LEN(dataP, step);
           memcpy(dataP + 1, SKIP_THE_FIRST_4_BYTES(valP), step - 1);
           dataP += step;
		   continue;
	   }

       /* If we are here we are processing 
	    * a long variable-length attribute. */
        dataP = (char*)ATT_ALIGN(dataP, relAttrs[i].align);
	    step = CUT_LAST_2_BITS_AND_TAKE_30_NEXT_BITS(valP);
	    memcpy(dataP, valP, step);
		dataP += step;
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
	int               rowHdSize;
	size_t            headerLen, dataLen, totalLen;
	Bool              hasnull;
	int               i;
    uint8*            nullBits;                  
    uint8*            bitPrev;
	int			      bitmask;
	char*             dataP;

    for (i = 0; i < relAttrsCount; i++)
	{
		if (isnull[i])
		{
			hasnull = True;
			continue;
		}

		if (relAttrs[i].len == -1 &&
			relAttrs[i].align == 'd' &&
			relAttrs[i].dimCount == 0 &&
			ARE_FIRST_2_BITS_ZEROS((char*)values[i]))
		{
			/* We need to check if an attribute needs flattening.
			 * Only composite types need flattening. 
			 * But we do not support composite attributes now. */
            continue;
		}
	}

	headerLen = offsetof(SRelRowHeader, nullBits);
	if (hasnull)
		headerLen += BITMAP_LEN(relAttrsCount);

	if (hasId)
		headerLen += sizeof(uint);

	headerLen = AlignDefault(headerLen);
    dataLen = computeRowSize(relAttrs, relAttrsCount, values, isnull);

    totalLen = headerLen + dataLen;

	rowHdSize = AlignDefault(sizeof(SRelRow));

	/* Allocate a new piece of memory for a new row. */
	row = (RelRow)_->memManager->alloc(rowHdSize + totalLen);
	row->data = rowHd = (RelRowHeader)((char*)row + rowHdSize);
	row->len = totalLen;

    SetBlockIdTo(&(row->self.block), InvalidBlockNumber);
	row->tblId = INVALID_TBLID;

	SET_FIRST_4_BYTES(rowHd, totalLen);
	SET_REL_ATTR_COUNT(rowHd, relAttrsCount);
    
	/* Offset means a position where data starts. */
	rowHd->offset = headerLen;

	/* dataP is a pointer to a position inside SRelRow structure,
	 * where we should write data. */
	dataP = (char*)rowHd + headerLen;
	nullBits = hasnull ? rowHd->nullBits : NULL;

    buildRelRow(relAttrs,
				relAttrsCount,
                values,
				isnull,
                dataP,
                dataLen,
				&(rowHd->mask),
                nullBits);

	return row;
}

void getDataFromRow(RelRow          row,
					int             relAttrsCount,
					RelAttribute    relAttrs,
					uint*           values,
				    Bool*           isnull)
{
	Bool         hasNulls        = row->data->mask & ROW_HASNULL != 0;
	int          savedAttrsCount = GET_REL_ATTR_COUNT(row->data);
	RelRowHeader rowHd           = row->data;
	int8*        nullsArr        = rowHd->nullBits;

	/* Inheritance situation is possible. So savedAttrsCount is the attributes
	 * count for a base type and relAttrsCount is a count for a child type.
	 * And relAttrsCount > savedAttrsCount. But we take only a common set of 
	 * attributes.
	 */
    int          attrsToExtract  = Min(relAttrsCount, savedAttrsCount);
	char*        dataP           = rowHd + rowHd->offset;
	long         offset          = 0;
	int          i;

	for (i = 0; i < attrsToExtract; i++)
	{
		RelAttribute  att = &relAttrs[i];

		if (hasNulls && IS_ATT_NULL(i, nullsArr))
		{
            values[i] = 0;
			isnull[i] = True;
			continue;   
		}

	    isnull[i] = False;
        offset    = ATT_ALIGN(offset, att->align);
        values[i] = ATT_RETRIEVE(dataP + offset, att->byVal, att->len);

        offset += SIZE_BY_LEN(att->len, dataP + offset);
    }

    for (; i < relAttrsCount; i++)
	{
        values[i] = 0;
        isnull[i] = True;
	}
} 

RelRow shortenRow(RelRow          row,
				  RelRow          oldRow,
				  int             relAttrsCount,
			      RelAttribute    relAttrs)
{
 	uint        values[ROW_ATTRS_MAX_COUNT];
    uint        oldValues[ROW_ATTRS_MAX_COUNT];
    int		    attrSizes[ROW_ATTRS_MAX_COUNT];

	Bool        isNulls[ROW_ATTRS_MAX_COUNT];
    Bool        oldIsNulls[ROW_ATTRS_MAX_COUNT];
    
    int         i;
	Bool	    hasNulls = False;
	char        attrActions[ROW_ATTRS_MAX_COUNT];
	size_t      offset;
	size_t		maxDataLen;

	getDataFromRow(row, relAttrsCount, relAttrs, values, isNulls);

	/* This is an auxiliary array which indicates attributes state. 
	 *   'd' is a default handling
	 *   'p' is being processed
	 */
    memset(attrActions, 'd', relAttrsCount* sizeof(char));

	for (i = 0; i < relAttrsCount; i++)
	{
		VarLenAttr  val = (VarLenAttr)(char*)values[i];

		if (isNulls[i])
		{
            attrActions[i] = 'p';
            hasNulls = True;
			continue;
		}

		if (relAttrs[i].len != -1)
		{
			attrActions[i] = 'p';
			continue;
		}

		/* If we are here relAttrs[i]->len == -1 */
	    if (relAttrs[i].storageStrategy == 'p')
			attrActions[i] = 'p';
            
        if (IS_FIRST_BYTE_1(val)) { /* We do not implement it here yet */ }
	    attrSizes[i] = ComputeSize(val);
    }

    offset = offsetof(SRelRowHeader, nullBits);
	if (hasNulls)
		offset += BITMAP_LEN(relAttrsCount);

	if (row->data->mask & ROW_HASID)
		offset += sizeof(uint);

	offset = AlignDefault(offset);

	/* At this moment offset is a length of a row header plus 
	 * null bits and plus is length.
	 */
	maxDataLen = MaxRowSize_By_RowsPerPage(ROWS_PER_PAGE) - offset;
    
	/* To compress a row we need to look for an attribute with storage
	 * strategy 'c' compressed. Also we can find large attributes with
	 * storage strategy 'c' or 's' and store it to a secondary table.
	 */
    while (computeRowSize(relAttrs,
					      relAttrsCount,
					      values,
					      isNulls) > maxDataLen)
	{
        /* Now it is not implemented yet. So we postpone it and 
		 * we will implement it further.
         */ 
	}

    return row;
}

/*

toast_insert_or_update(relation, tup, NULL, options);

Datum		toast_values[MaxHeapAttributeNumber];
bool		toast_isnull[MaxHeapAttributeNumber];

heap_deform_tuple(newtup, tupleDesc, toast_values, toast_isnull);

*/
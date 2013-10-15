
#include "relationmanager.h"

const SIRelationManager sRelationManager = 
{ 
	&sHashtableManager,
	createRelation,
	createRelationCache
};

const IRelationManager relationManager = &sRelationManager;

Hashtable relCache;

void createRelation(
	void*            self
	char*            relName, 
	uint             relTypeId, 
	uint             attrCount, 
	SRelAttribute*   attributes)
{
    IHashtableManager _ = (IHashtableManager)self;

	int               found;
	RelCacheItem      relItem;
    Relation          rel = (Relation)malloc(sizeof(SRelation));

	rel->refCount   = 1;
	rel->typeId     = relTypeId;
	rel->attrCount  = attrCount;
	rel->attributes = attributes;

	relItem = (RelCacheItem)_->hashInsert(_, relCache, (void*)&(rel->id));
	relItem->relation = rel;
}

void createRelationCache(void* self)
{
    IHashtableManager _ = (IHashtableManager)self;
	SHashtableSettings sett;

	memset(&sett, 0, sizeof(sett));

	sett.keyLen = sizeof(uint);
	sett.valLen = sizeof(SRelCacheItem) - sizeof(uint);
	sett.hashFunc = getHashId;

	relCache = _->createHashtable("Relation cache",
									400,
									&sett,
								    HASH_FUNC);
}

#define RelationRelation_Rowtype_Id  83

SRelAttribute Desc_pg_class[Natts_pg_class] = {catalog_relation};

formrdesc("pg_class", 
		  RelationRelation_Rowtype_Id, 
		  false,
		  true, 
		  Natts_pg_class, 
		  Desc_pg_class);

/* Suppose that for each column value we have 4 byte alignment.
 * In this case, for example, if we have a column of char(1) data type.
 * sizeof(char(1)) = 1 byte. So we have to waste 3 additional bytes.
 * 
 */
SRelRow createRelRow(void*           self,
					 int             relAttrsCount,
					 SRelAttribute*  relAttrs,
					 Bool		     hasId,
					 TupleDesc       tupleDescriptor,
				     Datum*          values,
				     Bool*           isnull)
{
	IHashtableManager _ = (IHashtableManager)self;
	RelRow            row;
	RelRowHeader      header;
	size_t            len;
	Bool              hasnull;
	int               i;
    for (i = 0; i < relAttrsCount; i++)
	{
		if (isnull[i])
			hasnull = true;
	}

	len = offsetof(SRelRowHeader, nullBits);
	if (hasnull)
		len += (relAttrsCount + 7) / 8;

	if (hasId)
		len += sizeof(uint);

	len = ALIGN(len);

	for (i = 0; i < numberOfAttributes; i++)
	{
		uint	 val;
		char*    valP;
		Bool     isPackable;

        if (isnull[i])
		    continue;

		val = values[i];
		valP = (char*)val;

		isPackable = relAttrs[i].len == -1 && relAttrs[i].storageStrategy != 'o';

        if (isPackable && VARATT_CAN_MAKE_SHORT(DatumGetPointer(val)))
		{
			len += SHORT_SIZE(valP);
		}
		else
		{
			if !(relAttrs[i].len == -1 && IS_1B(valP))
			{  
			   switch (relAttrs[i].align)// == 'i')
			   {
			   case 'i':
                   len = ALIGN_TYPE(ALIGN_INT, len);
			   case 'c':
				   len = (int)len;
			   case 'd':
				   len = ALIGN_TYPE(ALIGN_DOUBLE, len);
			   case 's':
                   len = ALIGN_TYPE(ALIGN_SHORT, len);
			   }
			}

			len = att_addlength_datum(data_length, 
				                      att[i]->attlen,
									  val);

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
	row->data = header = (RelRowHeader)((char*)row + ALIGN(sizeof(SRelRow)));
	row->len = len;

}

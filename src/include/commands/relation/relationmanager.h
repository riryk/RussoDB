
#ifndef RELATION_MANAGER_H
#define RELATION_MANAGER_H

#include "common.h"
#include "hashtable.h"

typedef struct SRelAttribute
{
   uint		 id;
   Name      name;
   uint      typeId;
   uint      typeParam;   //For varchar type it is the max value
   Bool      notNull;
   Bool      hasDefault;
   Bool      isInvisible;
   uint      collation;
} SRelAttribute;

/* Name convetion is applied: Any struct's name starts with 'S'
 * when a struct pointer can have an arbitrary name */
typedef struct SRelation
{
   uint            id;
   uint		       refCount;
   uint            typeId;
   uint            attrCount;
   SRelAttribute*  attributes;
} SRelation, *Relation;

/* This struct is used to insert a relation into a hash table */
typedef struct SRelCacheItem
{
   uint		       id;
   Relation	       relation;
} SRelCacheItem, *RelCacheItem;


typedef struct SRowFields
{
	uint          tranMin;		
	uint          tranMax;
	union
	{
		uint	  cmdId;		
		uint      tranVac;	
	}             field3;
} SRowFields, *RowFields;


typedef struct SDataFields
{
	uint		  len;
	uint		  typeMod;
	uint		  typeId;
} SDataFields, *DataFields;


typedef struct SRelRowHeader
{
	union
	{
		SRowFields   fiels;
		SDataFields  data;
	}          typeData;

	SRowPointer      curr;		
	uint16		     mask2;	
	uint16		     mask;	
	uint8		     hdrSize;
    uint8            nullBits;
} SRelRowHeader, *RelRowHeader;

typedef struct SRelRow
{ 
    uint             len;
    SRowPointer      self;
    uint		 	 tblId;
    SRelRowHeader    data;
} SRelRow, *RelRow


Hashtable RelationCache;

void createRelation(
	char*          relName, 
	uint           relTypeId, 
	uint           attrCount, 
	SRelAttribute* attributes);

void createRelationCache();

#endif
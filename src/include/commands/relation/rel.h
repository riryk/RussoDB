
#ifndef REL_H
#define REL_H

#include "common.h"

#define NATTS_MASK			0x07FF

#define ROW_HASNULL			0x0001	
#define ROW_HASVARWIDTH		0x0002	
#define ROW_HASEXTERNAL		0x0004	

typedef struct SRelAttribute
{
   uint		 id;
   Name      name;
   uint      typeId;
   uint      typeParam;   //For varchar type it is the max value
   int16     len;
   Bool      notNull;
   Bool      hasDefault;
   Bool      isInvisible;
   uint      collation;
   /* storageStrategy tells what to do with a row, when it does not fit 
    * into a page. The possible strategies are:
	*       'o', 'ordinary': value must be stored in common, ordinary, plain way.
	*                        It means that one part of the row, which fits into a table,
	*                        goes into one page, whereas another part goes to another part.
	*                        It can cause a large perfomance gap because we need to load 
	*                        two pages into memory.
	*       's': 'secondary' value will be stored in "secondary" table provided the table has one
    *       'c': 'compressed' value can be stored compressed inline
	*       'm': 'mixed' value can be stored compressed inline or in "secondary"
	*/
   char		 storageStrategy;
   char      align;
   Bool      byVal;
} SRelAttribute, *RelAttribute;

/* Name convetion is applied: Any struct's name starts with 'S'
 * when a struct pointer can have an arbitrary name */
typedef struct SRelation
{
   uint            id;
   uint		       refCount;
   uint            typeId;
   uint            attrCount;
   RelAttribute    attributes;
} SRelation, *Relation;

/* This struct is used to insert a relation into a hash table */
typedef struct SRelCacheItem
{
   uint		       id;
   Relation	       relation;
} SRelCacheItem, *RelCacheItem;

#endif

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


typedef struct SRelCacheItem
{
	uint		   id;
	Relation	   relation;
} SRelCacheItem;

Hashtable RelationCache;

void createRelation(
	char*          relName, 
	uint           relTypeId, 
	uint           attrCount, 
	SRelAttribute* attributes);


#endif
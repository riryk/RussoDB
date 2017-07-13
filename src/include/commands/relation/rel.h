
#ifndef REL_H
#define REL_H

#include "common.h"
#include "page.h"
#include "relfile.h"
#include "types.h"
#include "tuple.h"
#include "relattribute.h"

#define REL_MIN_FILL_PERCENT         10
#define REL_FILL_PERCENT_DEFAULT     100
#define INVALID_BLOCK 	             (0xFFFFFFFF)

/* Name convetion is applied: Any struct's name starts with 'S'
 * when a struct pointer can have an arbitrary name */
typedef struct SRelation
{
   SRelFileInfo    fileId;
   uint            id;
   uint		       refCount;
   uint            typeId;
   uint            attrCount;
   TupleDescriptor tupleDescriptor;
   RelAttribute    attributes;
   VarLenAttr      options;
   RelData         data;
   int             backendId;
   int			   storageManId;
} SRelation, *Relation;

/* This struct is used to insert a relation into a hash table */
typedef struct SRelCacheItem
{
   uint		       id;
   Relation	       relation;
} SRelCacheItem, *RelCacheItem;

typedef struct SAutoVacOptions
{
	Bool		enabled;
	int			vacMax;
	int			analyzeMax;
	int			vacCostDelay;
	int			vacCostLimit;
	int			freezeMinTime;
	int			freezeMaxTime;
	int			freezeTblTime;
	double		vacScaleFactor;
	double		analyzeScaleFactor;
} SAutoVacOptions, *AutoVacOptions;

typedef struct SRelOptions
{
	int		        len;	
	int			    fillPercent;	
	AutoVacOptions  autoVacOpts;	
	Bool		    security;
} SRelOptions, *RelOptions;

#define RelFillPercent(rel, defaultPercent) \
	((rel)->options ? \
	 (((RelOptions)((rel)->options))->fillPercent) : (defaultPercent))

#define GetPageFreeSpace(rel, defaultPercent) \
	(BlockSize * (100 - RelFillPercent((rel), (defaultPercent))) / 100)

#define RelGetCurrentBlock(rel) \
	((rel)->data != NULL ? \
     (rel)->data->currentBlock : INVALID_BLOCK)

#define RelationGetDescriptor(relation) ((relation)->tupleDescriptor)

#endif


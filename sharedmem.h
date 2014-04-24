
#include "common.h"

#ifndef SHARED_MEM_H
#define SHARED_MEM_H

typedef struct SSharMemHeader	
{
	int		    hdrId;			
	int		    procId;	
	size_t		totalSize;		/* total size of header */
	size_t		freeoffset;		/* offset to first free space */
	void*       index;			
} SSharMemHeader, *SharMemHeader;

#define SHAR_MEM_KEY_SIZE (48)

/* this is a hash bucket in the shmem index table */
typedef struct SSharMemItem
{
	char		key[SHAR_MEM_KEY_SIZE];	    /* string name */
	void*       location;		            /* location in shared mem */
	size_t		size;			            /* bytes allocated for the structure */
} SSharMemItem, *SharMemItem;


#define SHAR_MEM_NAME "Global\\PostgreSQL:"

#endif
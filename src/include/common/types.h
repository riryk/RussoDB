
#ifndef TYPES_H
#define TYPES_H

#include "common.h"

typedef struct SCol_1b
{
	uint8		header;
	char		data;		
} SCol_1b, *Col_1b;

typedef struct SCol_1b_e
{
	uint8		header;		
	uint8		len;		
	char		data;		
} SCol_1b_e, *Col_1b_e;

typedef union SCol_4b
{
	struct					
	{
		uint		header;
		char		data;
	} col_4byte;
	struct						
	{
		uint		header;
		uint		size; 
		char		data; 
	} compressed;
} SCol_4b, *Col_4b;

#define IS_4B(p) \
    ((((Col_1b)(p))->header & 0x03) == 0x00)

#define IS_1B(p) \
	((((Col_1b)(p))->header & 0x01) == 0x01)

#define IS_1B_E(p) \
	((((Col_1b)(p))->header) == 0x01)

#define VARSIZE_1B(p) \
	((((Col_1b)(p))->header >> 1) & 0x7F)

#define VARSIZE_1B_E(p) \
	(((Col_1b_e)(p))->len)

#define VARSIZE_4B(p) \
	((((Col_4b)(p))->col_4byte.header >> 2) & 0x3FFFFFFF)

#define CAN_MAKE_SHORT(p) \
	(IS_4B(p) && \
	 (VARSIZE_4B(p) - (int)sizeof(int) + 1) <= 0x7F)

#define SHORT_SIZE(p) \
	 (VARSIZE_4B(p) - (int)sizeof(int) + 1)

#define VARSIZE_ANY(PTR) \
	(VARATT_IS_1B_E(PTR) ? VARSIZE_1B_E(PTR) : \
	 (VARATT_IS_1B(PTR) ? VARSIZE_1B(PTR) : \
	  VARSIZE_4B(PTR)))

#define SET_VARSIZE_1B(p,len) \
	(((Col_1b)p)->header = (((uint8)len) << 1) | 0x01)

#define VARDATA_4B(p) \
	(((Col_4b)p)->col_4byte.data)


#endif

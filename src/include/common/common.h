
#include <stdlib.h>

#ifndef COMMON_H
#define COMMON_H

typedef unsigned int uint;
typedef int          Bool;

#define True         1
#define False        0

#define NAME_MAX_LENGTH  64

#define Max(x, y)		((x) > (y) ? (x) : (y))
#define Min(x, y)		((x) < (y) ? (x) : (y))
#define Abs(x)			((x) >= 0 ? (x) : -(x))

/* This is the same char* but the length restrictions is provided */
typedef struct SName
{
	char	   value[NAME_MAX_LENGTH];
} SName, *Name;


#define catalog_relation \
{ 1000, {"name"},      19, -1, False, False, False, 0 }, \
{ 1000, {"namespace"}, 26, -1, False, False, False, 0 }, \
{ 1000, {"type"},      26, -1, False, False, False, 0 }


#endif
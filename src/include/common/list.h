#include "nodes.h"
#include "string_info.h"

typedef struct SListCell
{
    union
	{
        void*      void_value;
		int        int_value;
        uint       uint_value;
	}           data;
    struct SListCell*  next;  
} SListCell, *ListCell;

typedef struct SList
{
	ENodeType      type;	
	int			   length;
	ListCell       head;
	ListCell       tail;
} SList, *List;

#define list_next(lc) ((lc)->next)
#define list_first(lc) ((lc)->data.ptr_value)

/* A macro to loop through the list */
#define foreach(cell, l)	\
	for ((cell) = getListHead(l); (cell) != NULL; (cell) = list_next(cell))

#define IsPointerList(l) ((l) == (List)NULL || isOfType((l), List))




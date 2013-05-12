
#include "hashtable.h"

#define MAX_REL_PARTS_NUMBER 10
#define RELATION_SEGMENT_SIZE 131072
#define BLOCK_SIZE 8192

typedef enum Fork
{
	INVALID_FORK = -1,
	MAIN_FORK = 0,
	FSM_FORK,
	VISIBILITYMAP_FORK,
	INIT_FORK
} Fork;


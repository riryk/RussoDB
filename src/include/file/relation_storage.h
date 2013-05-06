
#include "hashtable.h"

#define MAX_REL_PARTS_NUMBER 10

typedef enum Fork
{
	INVALID_FORK = -1,
	MAIN_FORK = 0,
	FSM_FORK,
	VISIBILITYMAP_FORK,
	INIT_FORK
} Fork;


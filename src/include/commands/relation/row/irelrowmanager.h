
#ifndef IRELROW_MANAGER_H
#define IRELROW_MANAGER_H

#include "common.h"
#include "relrow.h"

typedef struct SIRelRowManager
{       
	 RelRow (*createRelRow)(void*           self,
	 				        int             relAttrsCount,
	 				        RelAttribute    relAttrs,
	 				        Bool		    hasId,
	 			            uint*           values,
	 			            Bool*           isnull);

	 RelRow (*shortenRow)(RelRow          row,
	 			          RelRow          oldRow,
	 			          int             relAttrsCount,
	 		              RelAttribute    relAttrs);
     
} SIRelRowManager, *IRelRowManager;


#endif
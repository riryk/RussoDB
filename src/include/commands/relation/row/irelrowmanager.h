
#ifndef IRELROW_MANAGER_H
#define IRELROW_MANAGER_H

#include "common.h"

typedef struct SIRelRowManager
{       
	 RelRow (*createRelRow)(void*           self,
					        int             relAttrsCount,
					        RelAttribute    relAttrs,
					        Bool		    hasId,
				            uint*           values,
	 			            Bool*           isnull);

	 void (*shortenRow)(RelRow          row,
				        RelRow          oldRow,
				        int             relAttrsCount,
			            RelAttribute    relAttrs);

} SIRelRowManager, *IRelRowManager;


#endif
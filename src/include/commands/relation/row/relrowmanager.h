
#ifndef RELROW_MANAGER_H
#define RELROW_MANAGER_H

#include "common.h"
#include "relrow.h"
#include "rel.h"
#include "ihashtablemanager.h"
#include "irelrowmanager.h"

extern const SIRelRowManager sRelRowManager;
extern const IRelRowManager  relRowManager;

size_t computeRowSize(RelAttribute    relAttrs,
					  int             relAttrsCount,
					  uint*           values,
					  Bool*           isnull);

void buildRelRow(RelAttribute    relAttrs,
				 int             relAttrsCount,
                 uint*           values,
				 Bool*           isnull,
                 char*           dataP,
                 size_t          dataLen,
				 uint16*         mask,
                 uint8*          nullBits);

RelRow createRelRow(void*           self,
					int             relAttrsCount,
					RelAttribute    relAttrs,
					Bool 		    hasId,
				    uint*           values,
				    Bool*           isnull);

RelRow shortenRow(RelRow          row,
	  			  RelRow          oldRow,
				  int             relAttrsCount,
			      RelAttribute    relAttrs);

#endif
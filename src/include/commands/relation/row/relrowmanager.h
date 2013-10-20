
#ifndef RELROW_MANAGER_H
#define RELROW_MANAGER_H

#include "common.h"
#include "relrow.h"
#include "rel.h"
#include "ihashtablemanager.h"

RelRow createRelRow(void*           self,
					int             relAttrsCount,
					RelAttribute    relAttrs,
					Bool 		     hasId,
				    uint*           values,
				    Bool*           isnull);

#endif
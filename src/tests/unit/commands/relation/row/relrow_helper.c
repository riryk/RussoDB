
#include "relrow_helper.h"

void DoSetFixedSize(
    RelAttribute attr, 
	int16        len,
	char         align)
{
   attr->len = len;
   attr->align = align;
   attr->byVal = True;
}


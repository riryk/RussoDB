#ifndef Brin_Page_Operations_h
#define Brin_Page_Operations_h

#include "buffer.h"

Bool brinCanDoSamePageUpdate(BufferId buffer, size_t originalSize, size_t newSize);

#endif
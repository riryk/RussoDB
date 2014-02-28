
#include "ilatchmanager.h"

#ifndef LATCHMANAGER_H
#define LATCHMANAGER_H

extern const SILatchManager sLatchManager;
extern const ILatchManager  latchManager;

void initLatch(void* self, Latch latch);
void setLatch(Latch latch);

#endif

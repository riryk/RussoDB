
#include "ilatchmanager.h"

#ifndef LATCHMANAGER_H
#define LATCHMANAGER_H

extern const SILatchManager sLatchManager;
extern const ILatchManager  latchManager;

Latch initLatch(void* self);
void setLatch(Latch latch);
void resetLatch(Latch latch);
void waitLatch(void* self, Latch latch);

#endif

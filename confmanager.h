
#include "iconfmanager.h"

#ifndef CONFMANAGER_H
#define CONFMANAGER_H

Bool  getIsPostmaster();
void  setIsPostmaster(Bool isPostmaster);

OutputDestination getOutputDest();
void              setOutputDest(OutputDestination dest);

#endif

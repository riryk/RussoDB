
#include "iconfmanager.h"

#ifndef CONFMANAGER_H
#define CONFMANAGER_H

extern const SIErrorLoggerConfManager sErrorLoggerConfManager;
extern const IErrorLoggerConfManager  errorLoggerConfManager;

extern const SIConfManager sConfManager;
extern const IConfManager  confManager;

Bool  getIsPostmaster();
void  setIsPostmaster(Bool isPostmaster);

OutputDestination getOutputDest();
void              setOutputDest(OutputDestination dest);

int  getMinClientLogLevel();
void setMinClientLogLevel(int level);

int  getMinLogLevel();
void setMinLogLevel(int level);

#endif

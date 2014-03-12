
#ifndef ICONFMANAGER_H
#define ICONFMANAGER_H

#include "error.h"

typedef struct SIErrorLoggerConfManager
{
    int   (*getMinLogLevel)();
    void  (*setMinLogLevel)(int level);

    int   (*getMinClientLogLevel)();
    void  (*setMinClientLogLevel)(int level);
} SIErrorLoggerConfManager, *IErrorLoggerConfManager;


typedef struct SIConfManager
{
	IErrorLoggerConfManager  errLogConfgMan;

	Bool  (*getIsPostmaster)();
    void  (*setIsPostmaster)(Bool isPostmaster);

    OutputDestination (*getOutputDest)();
	void              (*setOutputDest)(OutputDestination dest);
} SIConfManager, *IConfManager;
 
#endif
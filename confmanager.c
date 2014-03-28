#include "confmanager.h"

Bool                isPostmaster;
OutputDestination   outputDest; 

const SIErrorLoggerConfManager sErrorLoggerConfManager = 
{ 
	getMinLogLevel,
    setMinLogLevel,
    getMinClientLogLevel,
    setMinClientLogLevel
};

const IErrorLoggerConfManager errorLoggerConfManager = &sErrorLoggerConfManager;

const SIConfManager sConfManager = 
{ 
    &sErrorLoggerConfManager,
    getIsPostmaster,
    setIsPostmaster,
    getOutputDest,
    setOutputDest
};

const IConfManager confManager = &sConfManager;

Bool getIsPostmaster()
{
   return isPostmaster;
}

void setIsPostmaster(Bool pisPostmaster)
{
   isPostmaster = pisPostmaster;
}

OutputDestination getOutputDest()
{
   return outputDest; 
}

void setOutputDest(OutputDestination dest)
{
   outputDest = dest;
}
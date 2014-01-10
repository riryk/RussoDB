#include "confmanager.h"

Bool                isPostmaster;
OutputDestination   outputDest; 

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
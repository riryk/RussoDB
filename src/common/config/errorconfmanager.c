
#include "confmanager.h"

int  minLogLevel;
int  minClientLogLevel;

int  getMinLogLevel() 
{ 
	return minLogLevel; 
}

void setMinLogLevel(int level) 
{ 
	minLogLevel = level; 
}

int getMinClientLogLevel() 
{
    return minLogLevel; 
}

void setMinClientLogLevel(int level)
{
    minClientLogLevel = level;
}
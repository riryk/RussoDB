#include "iconfmanager.h"
#include "imemcontainermanager.h"

#ifndef IERRORLOGGER_H
#define IERRORLOGGER_H

typedef struct SIErrorLogger
{
	IConfManager   confManager;
	IStringManager strManager;
	void*          memContManager;

	void (*assertArg)(Bool condition);	
	void (*assert)(Bool condition);
	void (*log)(int level, int code, char* message,...); 
} SIErrorLogger, *IErrorLogger;

#endif
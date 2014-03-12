
#include "iconfmanager.h"
#include "string_info.h"

#ifndef IERRORLOGGER_H
#define IERRORLOGGER_H

typedef struct SIErrorLogger
{
	IConfManager   confManager;
	IStringManager strManager;
	void*          logger;
	void*          memContManager;

	void (*assertArg)(Bool condition);	
	void (*assert)(Bool condition);
	void (*log)(int level, int code, char* message,...); 
} SIErrorLogger, *IErrorLogger;

#endif
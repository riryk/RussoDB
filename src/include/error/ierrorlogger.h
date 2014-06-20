
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

	void (*writeException)(
           char*    condName,
           char*    errType,
	       char*    fileName,
	       int      lineNum);

	void (*writeMessageInChunks)(
           void*    self,
           char*    data, 
	       int      len);

	Bool (*beginError)(
           void*    self,
           int      level, 
           char*    filename, 
           int      linenum,
           char*    funcname, 
           char*    domain);
    
	void (*endError)(
           void*    self,
	       int      dummy,
	       ...);

	void (*ctorErrorLogger)(void*  self);

} SIErrorLogger, *IErrorLogger;

#endif
#include "ilatchmanager.h"
#include "ierrorlogger.h"
#include "string_info.h"
#include "imemcontainermanager.h"
#include "ilistmanager.h"
#include "iprocesshelper.h"
#include "isignalmanager.h"

#ifndef ILOGGER_H
#define ILOGGER_H

typedef struct SILogger
{
    ILatchManager         latchManager;
    IErrorLogger          errorLogger;
    IStringManager        stringManager;
    IMemoryManager        memManager;
    IListManager          listManager;
    IProcessManager       processManager;
	ISignalManager        signalManager;

	void (*ctorLogger)(void* self, char* logDir);

	void (*write_message_file)(
	       char*       buffer, 
	       int         count);
    
	void (*logger_start)(void*  self);

	void (*logger_main) (void*  self);

	void (*processLogBuffer)(
	       void*       self,
           char*       buf, 
	       int*        buf_bytes);

	char* (*getLogFileName)(
           void*       self,
	       int         time);

	FILE* (*logFileOpen)(
           void*       self,
	       char*       filename,
	       char*       mode);

} SILogger, *ILogger;

#endif
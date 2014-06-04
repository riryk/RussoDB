#include "ilatchmanager.h"
#include "ierrorlogger.h"
#include "string_info.h"
#include "imemcontainermanager.h"
#include "ilistmanager.h"
#include "iprocesshelper.h"

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

	void (*ctorLogger)(void* self, char* logDir);

	void (*write_message_file)(
	     char*               buffer, 
	     int                 count);
    
	void (*logger_start)(void*  self);
	void (*logger_main) (void*  self);

} SILogger, *ILogger;

#endif
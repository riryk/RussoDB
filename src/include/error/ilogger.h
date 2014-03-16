#include "ilatchmanager.h"
#include "ierrorlogger.h"
#include "string_info.h"
#include "imemcontainermanager.h"
#include "ilistmanager.h"

#ifndef ILOGGER_H
#define ILOGGER_H

typedef struct SILogger
{
    ILatchManager         latchManager;
    IErrorLogger          errorLogger;
    IStringManager        stringManager;
    IMemoryManager        memManager;
    IListManager          listManager;

	void (*write_message_file)(
	     char*               buffer, 
	     int                 count);

} SILogger, *ILogger;

#endif
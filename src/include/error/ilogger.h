#include "ilatchmanager.h"
#include "ierrorlogger.h"

#ifndef ILOGGER_H
#define ILOGGER_H

typedef struct SILogger
{
    ILatchManager  latchManager;
    IErrorLogger   errorLogger;

	void (*write_message_file)(
	     char*               buffer, 
	     int                 count);

} SILogger, *ILogger;

#endif
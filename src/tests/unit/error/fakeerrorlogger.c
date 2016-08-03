#include "ierrorlogger.h"
#include "fakeerrorlogger.h"

const SIErrorLogger sFakeErrorLogger = 
{
    NULL,
    NULL,
    NULL,
    NULL,
    fakeAssertArg,
    fakeAssert,
    fakeLog
};

uint assertArgFails = 0;
uint assertFails    = 0;
uint logMessages    = 0;
uint errorMessages  = 0;
uint exceptions     = 0;

void fakeAssertArg(Bool condition)
{
    if (!condition)
       assertArgFails++;
}

void fakeAssert(Bool condition)
{
    if (!condition)
       assertFails++;
}

void fakeLog(int level, int code, char* message,...)
{
	if (level == LOG_ERROR)
        errorMessages++;

	if (level == LOG_LOG)
        logMessages++;
}


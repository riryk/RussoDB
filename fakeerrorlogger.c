#include "ierrorlogger.h"
#include "fakeerrorlogger.h"

void assertArg(Bool condition);	
void assert(Bool condition);
void log(int level, int code, char* message,...); 

const SIErrorLogger sFakeErrorLogger = 
{
    NULL,
    NULL,
    NULL,
    NULL,
    assertArg,
    assert,
    log
};

uint assertArgFails = 0;
uint assertFails    = 0;
uint logMessages    = 0;
uint errorMessages  = 0;
uint exceptions     = 0;

void assertArg(Bool condition)
{
    if (!condition)
       assertArgFails++;
}

void assert(Bool condition)
{
    if (!condition)
       assertFails++;
}

void log(int level, int code, char* message,...)
{
	if (level == LOG_ERROR)
        errorMessages++;

	if (level == LOG_LOG)
        logMessages++;
}


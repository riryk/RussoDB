#include "ierrorlogger.h"

void assertArg(Bool condition);	
void assert(Bool condition);
void log(int level, int code, char* message,...); 

const SIErrorLogger sFakeErrorLogger = 
{
    NULL,
    assertArg,
    assert,
    log
};

uint assertArgFails = 0;
uint assertFails    = 0;

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

}

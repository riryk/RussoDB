#include "ierrorlogger.h"

extern const SIErrorLogger sFakeErrorLogger;

extern uint assertArgFails;
extern uint assertFails;
extern uint logMessages; 
extern uint errorMessages;

void fakeAssertArg(Bool condition);	
void fakeAssert(Bool condition);
void fakeLog(int level, int code, char* message,...); 

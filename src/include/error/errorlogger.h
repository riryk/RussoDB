
#include "common.h"
#include "ierrorlogger.h"

#ifndef ERRORLOGGER_H
#define ERRORLOGGER_H

extern const SIErrorLogger sErrorLogger;
extern const IErrorLogger errorManager;

void reThrowError(void* self);
void assertCond(Bool condition);
void assertArg(Bool condition);
void assert(Bool condition);
void emitError(void* self);
void log(int level, int code, char* message,...);
void writeException(
     char*    condName,
     char*    errType,
	 char*    fileName,
	 int      lineNum);
void writeMessageInChunks(
     void*    self,
     char*    data, 
	 int      len);

#endif
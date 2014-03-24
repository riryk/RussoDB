
#ifndef ERRORLOGGER_H
#define ERRORLOGGER_H

#include "common.h"

void reThrowError(void* self);
void assertCond(Bool condition);
void emitError(void* self);
void writeException(
     char*    condName,
     char*    errType,
	 char*    fileName,
	 int      lineNum);

#endif
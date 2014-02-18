
#ifndef ERRORLOGGER_H
#define ERRORLOGGER_H

void reThrowError(void* self);
void assertCond(Bool condition);
void emitError(void* self);

#endif
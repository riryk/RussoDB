#include "ierrorlogger.h"
#include "snprintf.h"

#ifndef ISTRING_INFO_H
#define ISTRING_INFO_H

typedef struct SStringInfo
{
	char*       data;
	int			len;
	int			maxlen;
	int			cursor;
} SStringInfo, *StringInfo;

typedef struct SIStringManager
{
    IMemoryManager  memManager;
	void*           errorLogger;

    void (*initStringInfo)(void* self, StringInfo str);
} SIStringManager, *IStringManager;

#endif
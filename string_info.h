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

	void (*appendStringInfo)(
         void*           self,
	     StringInfo      strinf, 
	     char*           str,
	     ...);

	void (*appendWithTabs)(
	     void*           self,
         StringInfo      buf,
	     char*           str);

	void (*appendStringInfoChar)(
         void*           self,
	     StringInfo      str, 
	     char            ch);

} SIStringManager, *IStringManager;

void appendStringInfo(
    void*           self,
	StringInfo      strinf, 
	char*           str,
	...);

#endif
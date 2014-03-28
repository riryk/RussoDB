#include "snprintf.h"
#include "imemorymanager.h"

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
    
	void (*appendStringInfoBinary)(
	     void*           self,
	     StringInfo      str,
	     char*           data,
	     int             datalen);

} SIStringManager, *IStringManager;

void initStringInfo(
	void*             self,
	StringInfo        str);

void appendStringInfo(
    void*           self,
	StringInfo      strinf, 
	char*           str,
	...);

void appendStringInfoBinary(
	void*             self,
	StringInfo        str,
	char*             data,
	int               datalen);

void appendStringInfoChar(
    void*             self,
	StringInfo        str, 
	char              ch);

void appendWithTabs(
	void*             self,
    StringInfo        buf,
	char*             str);

#endif
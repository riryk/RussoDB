
#include "string_info.h"


void initStringInfo(
	void*             self,
	StringInfo        str)
{
	IStringManager    _  = (IStringManager)self;
	IMemoryManager    mm = _->memManager;

	/* Initial default buffer size. */
	int			size = 1024;

	str->data    = (char*)mm->alloc(size);
	str->maxlen  = size;

	str->data[0] = '\0';
	str->len     = 0;
	str->cursor  = 0;
}

Bool appendStringInfoBase(
    void*           self,
	StringInfo      str, 
	char*           fmt, 
	va_list         args)
{
    IStringManager _ = (IStringManager)self;
    

	int		 avail;
	int      nprinted;

    Assert(str != NULL);
}

/* Add new string to an existed string.
 */
void appendStringInfo(
	StringInfo      str, 
	char*           fmt,
	...)
{
	CYCLE
	{
        va_list		args;
		Bool		success;  
         
        va_start(args, fmt);
		success = 
        va_end(args);
	}

	for (;;)
	{
		va_list		args;
		bool		success;

		/* Try to format the data. */
		va_start(args, fmt);
		success = appendStringInfoVA(str, fmt, args);
		va_end(args);

		if (success)
			break;

		/* Double the buffer size and try again. */
		enlargeStringInfo(str, str->maxlen);
	}
}



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
	StringInfo      strinf, 
	char*           str, 
	va_list         args)
{
    IStringManager _    = (IStringManager)self;
	IErrorLogger   elog = (IErrorLogger)_->errorLogger;

	int		 avail;
	int      nprinted;

	ASSERT(elog, strinf != NULL, False); 

    /* There is very little free space. We do not bother 
	 * trying and just return.
	 */
	avail = strinf->maxlen - strinf->len - 1;
	if (avail < 16)
		return False;

	snprintf_args(
		strinf->data + strinf->len,
		avail,
		str,
		args);

	ASSERT(elog, strinf->data[strinf->maxlen - 1] == '\0', False);

	if (nprinted >= 0 && nprinted < avail - 1)
	{
		/* In this case we have successfully added a string.
		 * We recalculate the string length.
		 */
		strinf->len += nprinted;
		return True;
	}

	/* In case of a failure we mark the end of the string
	 * and return false.
	 */
	strinf->data[strinf->len] = '\0';
	return False;
}

void enlargeStringInfo(
    void*           self,
	StringInfo      strinf, 
	int             needed_size)
{
	IStringManager _    = (IStringManager)self;
	IErrorLogger   elog = (IErrorLogger)_->errorLogger;

    int  newlen;

	if (needed_size < 0)	
	{
		elog->log(LOG_ERROR,
		          ERROR_CODE_INVALID_PARAM,
				  "invalid string enlargement size: %d",
				  needed_size);
	}

	/* The needed size is too large */
	if (needed_size >= MAX_ALLOC_SIZE - (size_t)strinf->len)
	{
		elog->log(LOG_ERROR,
			      ERROR_CODE_MAX_ALLOC_MEM_EXCEEDED,
				  "Cannot enlarge string buffer containing %d bytes by %d more bytes.",
				  strinf->len,
				  needed_size);
	}

	/* Now needed_size contains the whole string length */
	needed_size += strinf->len + 1;		

	/* If our string already has enough free space,
	 * we just return.
	 */
	if (needed_size <= strinf->maxlen)
        return;

	/* We do not bother allocating little space.
	 * For perfomance reason we double space.
	 */
	newlen = 2 * strinf->maxlen;

	/* But if needed space is very large 
	 * it may be not enough. So we continue
	 * doubling the space until we cover the need.
	 */
	while (needed > newlen)
		newlen = 2 * newlen;

	if (newlen > MAX_ALLOC_SIZE)
		newlen = MAX_ALLOC_SIZE;


}

/* Add new string to an existed string.
 */
void appendStringInfo(
    void*           self,
	StringInfo      strinf, 
	char*           str,
	...)
{
	CYCLE
	{
        va_list		args;
		Bool		success;  
         
        va_start(args, fmt);

		success = appendStringInfoBase(
			 self,
			 strinf,
			 str,
			 args);

        va_end(args);

		if (success)
			break;
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


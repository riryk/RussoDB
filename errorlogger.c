
#include <setjmp.h>
#include "common.h"

jmp_buf*  exceptionStack = NULL;
int	      stackDepth     = -1;


Bool beginerror(
    int           level, 
    char*         filename, 
    int           linenum,
    char*         funcname, 
    char*         domain)
{
	int         i;

	Bool		writeToServer;
	Bool		writeToClient = False;

	if (level >= LOG_ERROR)
	{
        if (CritSectionCount > 0)
			level = LOG_PANIC;

		if (level == LOG_ERROR && exceptionStack == NULL)
			level = LOG_FATAL;

		/* Go through the previous errors in the error stack 
		 * and find an error with severer error level and 
		 * update our current error level.
		 */
		for (i = 0; i <= stackDepth; i++)
			level = Max(elevel, errordata[i].elevel);
	}

	/*
	 * Now decide whether we need to process this report at all; if it's
	 * warning or less and not enabled for logging, just return FALSE without
	 * starting up any error logging machinery.
	 */

    /* Determine whether message is enabled for server log output */
	if (IsPostmaster)
		writeToServer = is_log_level_output(elevel, log_min_messages);
	else
		/* In bootstrap/standalone case, do not sort LOG out-of-order */
		writeToServer = (elevel >= log_min_messages);

}
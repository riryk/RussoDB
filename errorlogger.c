
#include <setjmp.h>
#include "common.h"

jmp_buf*  exceptionStack = NULL;
int	      stackDepth     = -1;

/* checks if errorLevel is logically more 
 * than min error level or not. 
 *
 * Generally this is the right thing for testing
 * whether a message should go to the postmaster log.
 */
Bool compareErrorLevels(int errorLevel, int minErrorLevel)
{
	Bool isLog         = errorLevel == LOG_LOG;
	Bool isCommErr     = errorLevel == LOG_COMMUNICATION_ERROR;
    Bool moreThanFatal = errorLevel >= LOG_FATAL;

	Bool isMinLessThanError = minErrorLevel <= LOG_ERROR;
    Bool isMinEqualToLog    = minErrorLevel == LOG_LOG;

	/* LOG messages and COMMUNICATION errors 
	 * are always send to the server unless min error
	 * level is more than ERROR. 
	 */
    if (isLog || isCommErr)
	{
        if (isMinLessThanError)
			return True;
	}
	else if (isMinEqualToLog)
	{
        if (moreThanFatal)
			return True;
	}
	else if (errorLevel >= minErrorLevel)
		return True;

	return False;
}

Bool beginerror(
    void*         self,
    int           level, 
    char*         filename, 
    int           linenum,
    char*         funcname, 
    char*         domain)
{
    IErrorLogger             _   = (IErrorLogger)self;
    IErrorLoggerConfManager  em  = _->errLogConfgMan;    

	int           i;

	Bool		  writeToServer;
	Bool		  writeToClient = False;

	int           minLogLevel   = em->getMinLogLevel();

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
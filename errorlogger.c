
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

Bool beginError(
    void*         self,
    int           level, 
    char*         filename, 
    int           linenum,
    char*         funcname, 
    char*         domain)
{
    IErrorLogger             _   = (IErrorLogger)self;
    IConfManager             cm  = _->confManager;;
    IErrorLoggerConfManager  em  = _->errLogConfgMan;    

	int                i;

	Bool		       writeToServer  = False;
	Bool		       writeToClient  = False;

	int                minLogLevel    = em->getMinLogLevel();
    int                minClientLevel = em->getMinClientLogLevel()

	Bool               isPostmaster   = cm->getIsPostmaster();
    OutputDestination  outputDest     = cm->getOutputDest();

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

	writeToServer = IsPostmaster ? 
		compareErrorLevels(level, minLogLevel) :
	    (level >= minLogLevel);

	if (outputDest == OutputRemote && level != LOG_COMMUNICATION_ERROR)
        writeToClient = (level >= minClientLevel || level == LOG_INFO);
	
    /* Skip processing effort if non-error message will not be output */
	if (level < LOG_ERROR && !writeToServer && !writeToClient)
		return False;
}


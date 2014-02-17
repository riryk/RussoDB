
#include <setjmp.h>
#include "common.h"
#include "error.h"
#include "ierrorlogger.h"
#include "string.h"
#include "string_info.h"

jmp_buf*          exceptionStack = NULL;
int	              stackDepth     = -1;
int	              recursDepth    = 0;	
SErrorInfo        errorInfos[ERROR_STACK_SIZE];
ErrorCallback*    errorStack     = NULL; 

char  str_start_time[TIME_STRING_LEN];
char  str_log_time[TIME_STRING_LEN];

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
    if ((isLog || isCommErr) && isMinLessThanError)
	    return True;

	if (isMinEqualToLog && moreThanFatal)
	    return True;

	if (errorLevel >= minErrorLevel)
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
    IConfManager             cm  = _->confManager;
    IErrorLoggerConfManager  em  = cm->errLogConfgMan;    
	IMemContainerManager     mm  = _->memContManager;

	int                i;
	ErrorInfo          einf;

	Bool		       writeToServer  = False;
	Bool		       writeToClient  = False;

	int                minLogLevel    = em->getMinLogLevel();
    int                minClientLevel = em->getMinClientLogLevel();

	Bool               isPostmaster   = cm->getIsPostmaster();
    OutputDestination  outputDest     = cm->getOutputDest();

	if (level >= LOG_ERROR)
	{
        /*if (CritSectionCount > 0)
			level = LOG_PANIC;*/

		if (level == LOG_ERROR && exceptionStack == NULL)
			level = LOG_FATAL;

		/* Go through the previous errors in the error stack 
		 * and find an error with severer error level and 
		 * update our current error level.
		 */
		for (i = 0; i <= stackDepth; i++)
			level = Max(level, errorInfos[i].level);
	}

	writeToServer = IsPostmaster ? 
		compareErrorLevels(level, minLogLevel) :
	    (level >= minLogLevel);

	if (outputDest == OutputRemote && level != LOG_COMMUNICATION_ERROR)
        writeToClient = (level >= minClientLevel || level == LOG_INFO);
	
    /* Skip processing effort if non-error message will not be output */
	if (level < LOG_ERROR && !writeToServer && !writeToClient)
		return False;

	if (recursDepth++ > 0 && level >= ERROR)
	{
		/* Here an error has occured during an error processing
		 * We clear all previous error info and reduce 
		 * ErrorContainer memory size to 8K. It will be enough
		 * to report even OutOfMemory error
		 */
		mm->resetErrCont(mm);

		/* Prevent infinite recursion. */
        if (recursDepth > 2)
            errorStack = NULL;
	}

	if (++stackDepth >= ERROR_STACK_SIZE)
	{
        /* Stack is not big enough.  
		 * It can also be an infinite recursion.
		 * Report a panic error.
		 */
		stackDepth = -1;	/* make room on stack */
		//ereport(PANIC, (errmsg_internal("ERRORDATA_STACK_SIZE exceeded")));
	}

	/* Initialize data for this error frame */
	einf = &errorInfos[stackDepth];
	MemSet(einf, 0, sizeof(SErrorInfo));

	einf->level          = level;
	einf->reportToServer = writeToServer;
	einf->reportToClient = writeToClient;

	if (filename != NULL)
	{
		/* Keep only base name */
		char* slash = strrchr(filename, '/');

		if (slash != NULL)
			filename = slash + 1;
	}

	einf->fileName = filename;
	einf->lineNum  = linenum;
	einf->funcName = funcname;
    
	einf->domain = (domain != NULL) ?
                domain :
                "postgres";

	einf->errorCode = ERROR_CODE_SUCCESS;

	if (level >= LOG_ERROR)
		einf->errorCode = ERROR_CODE_INTERNAL_ERROR;

    if (level == LOG_WARNING)
        einf->errorCode = ERROR_CODE_WARNING;

	einf->savedError = errno;

	recursDepth--;
	return True;
}

/* endError completes error reporting and 
 * clears error stack.
 */
void endError(
    void*           self,
	int             dummy,
	...)
{
	IErrorLogger         _  = (IErrorLogger)self;
    IMemContainerManager mm = _->memContManager;

	ErrorInfo       einf  = &errorInfos[stackDepth];
	int             level = einf->level; 

	MemoryContainer oldContainer;
    ErrorCallback   errcal;

	recursDepth++;
    
    if (stackDepth < 0)
	{
	    stackDepth = -1;
		// ereport(ERROR, (errmsg_internal("errstart was not called"))); 
	}
    
	oldContainer = mm->changeToErrorContainer();

	/* Call all callback functions */
	for (errcal = errorStack; 
		 errcal != NULL; 
		 errcal = errcal->prev)
	{
		(*errcal->callback)(errcal->arg);
	}

	if (level == LOG_ERROR)
	{
        recursDepth--;
		reThrowError(self);
		exit(0);
	}
}

void errorInfoToString(
    StringInfo         buf, 
	ErrorInfo          einf)
{
	long   line_number = 0;
	int	   my_pid      = 0;

	int	   format_len;
	int	   i;
    
	line_number++;

	if (Log_line_prefix == NULL)
		return;	


}

char* severity_message(int level)
{
	char* prefix;

	switch (level)
	{
		case DEBUG1:
		case DEBUG2:
		case DEBUG3:
		case DEBUG4:
		case DEBUG5:
			prefix = "DEBUG";
			break;
		case LOG:
		case COMMERROR:
			prefix = "LOG";
			break;
		case INFO:
			prefix = "INFO";
			break;
		case NOTICE:
			prefix = "NOTICE";
			break;
		case WARNING:
			prefix = "WARNING";
			break;
		case ERROR:
			prefix = "ERROR";
			break;
		case FATAL:
			prefix = "FATAL";
			break;
		case PANIC:
			prefix = "PANIC";
			break;
		default:
			prefix = "???";
			break;
	}

	return prefix;
}

void sendToServer(
    void*        self,
	ErrorInfo    einf)
{
    IErrorLogger   _  = (IErrorLogger)self;
	IStringManager sm = _->strManager;

    SStringInfo  buf;

	sm->initStringInfo(sm, &buf);

	str_log_time[0] = '\0';

	sm->appendStringInfo(sm, &buf, "%s:  ", severity_message(einf->level));
	
	if (einf->message != NULL)
		sm->appendWithTabs(sm, &buf, einf->message);
	else 
        sm->appendWithTabs(sm, &buf, "missing error text");


}

void sendToClient(ErrorInfo  einf)
{
     
}

void emitError(void*  self)
{
	IErrorLogger          _  = (IErrorLogger)self;
	IMemContainerManager  mm = _->memContManager

    MemoryContainer  oldContainer;
    ErrorInfo        einf = errorInfos[errordata_stack_depth];

    recursDepth++;
    
	if (stackDepth < 0)
	{
	    stackDepth = -1;
		// ereport(ERROR, (errmsg_internal("errstart was not called"))); 
	}

	oldContainer = mm->changeToErrorContainer();  

	/* Report error to server log */
	if (einf->reportToServer)
		sendToServer(einf);

	if (einf->reportToClient)
		sendToClient(einf);
}

/* ExceptionalCondition - Handles the failure of an Assert() */
void writeException(
    char*    condName,
    char*    errType,
	char*    fileName,
	int      lineNum)
{
    if (condName == NULL || fileName == NULL || errType == NULL)
	{
	    fprintf(stderr, "writeException: bad arguments\n");
		return;
	}

	fprintf(stderr, 
		"writeException: %s(\"%s\", File: \"%s\", Line: %d)\n",
        errType,
        condName,
        fileName,
        lineNum);
}

void assertCond(Bool condition)
{ }

void reThrowError(void*  self)
{
	IErrorLogger _ = (IErrorLogger)self;

	/* If possible, throw the error to the next outer setjmp handler */
	if (exceptionStack != NULL)
	{
		/* Restores the environment to the state indicated by env, 
		 * evaluating the setjmp expression that filled env as val.
         * The function never returns to the point where it has been invoked. 
		 * Instead, the function transfers the control to the point 
		 * where setjmp was last used to fill the env, 
		 * and evaluates the whole expression as val 
		 * (unless this is zero, in which case it evaluates as value of 1).
		 */
		longjmp(*exceptionStack, 1);

	    /* setjmp has not been called before.
		 * Report an error.
		 */ 
	    writeException(
			"reThrowError tried to return", 
			"assertion failed",
			__FILE__, 
			__LINE__);

		return;
	}

    /* The error was thrown in TRY block. So we should promote it 
	 * to fatal and report.
	 */
	ErrorInfo einf = &errorInfos[stackDepth];

	assertCond(stackDepth >= 0);
	assertCond(einf->level == LOG_ERROR);

	einf->level = LOG_FATAL;

    einf->reportToServer = IsPostmaster ? 
		compareErrorLevels(level, minLogLevel) :
	    (level >= minLogLevel);

	if (outputDest == OutputRemote)
		einf->reportToClient = (LOG_FATAL >= minLogLevel);

	errorStack = NULL;
    
	endError(_, 0);
}



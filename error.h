

#define LOG_ERROR		20		/* user error - abort transaction; 
                                 * return to known state */

#define LOG_FATAL		21		/* fatal error - abort process */
#define LOG_PANIC		22		/* take down the other backends with me */


/* The structure contains complete information 
 * about an error
 */
typedef struct SErrorInfo
{
	int			level;			      /* error level */
	Bool		reportToServer;		  /* whether to report to server log or not */
	Bool		reportToClient;		  /* whether to report to client log or not */
	Bool		includeFuncName;	  /* whether to include a function's name or not */
	Bool		includeStatement;	  /* whether to include a statement ot not */

	char*       fileName;		      /* returns the file's name where an error occured 
									   * __FILE__ macros returns this file */

	int			lineNum;			  /* returns the line where an error occured
									   * __LINE__ macros returns this line */

	char*       funcName;		      /* returns the function name where an error occured
									   * __func__ of ereport() call */

	char*       domain;			      /* domain */
	int			errorCode;		      /* error state */
	char*       message;		      /* error message */
	char*       detailedMessage;	  /* detailed error message */
	char*       detailedLogMessage;	  /* detailed error message for server log */
	char*       hintMessage;		  /* hint message */
	char*       contextMessage;		  /* context message */
	int			cursorPosition;		  /* cursor position into query string */
	int			internalPosition;	  /* cursor index into internal query */
	char*       internalQuery;	      /* text of internally-generated query */
	int			savedError;	          
} SErrorInfo, *ErrorInfo
#include "common.h"

#ifndef ERROR_H
#define ERROR_H

#define ERROR_STACK_SIZE  5

typedef enum
{
	OutputNone,					/* output is discarded */
	OutputDebug,				/* output is sent to debugging output */
	OutputRemote				/* results sent to frontend process */
} OutputDestination;


/* Error level codes */
#define LOG_DEBUG5		            10			
#define LOG_DEBUG4		            11
#define LOG_DEBUG3		            12
#define LOG_DEBUG2		            13
#define LOG_DEBUG1		            14			

#define LOG_LOG			            15	    /* Server messages. Send only to server */


#define LOG_COMMUNICATION_ERROR	    16	    /* Client communication problem. 
                                             * Simply log it to the server. */

#define LOG_INFO		            17	    /* Messages that are sent to client. */

#define LOG_NOTICE		            18	    /* Helpful messages to users about query
								             * operation; sent to client and server log by
								             * default. */

#define LOG_WARNING		            19	    /* Warnings.  NOTICE is for expected message
								             * WARNING is for unexpected messages. */

#define LOG_ERROR		            20		/* user error - abort transaction; 
                                             * return to known state */

#define LOG_FATAL		            21		/* fatal error - abort process */
#define LOG_PANIC		            22		/* take down the other backends with me */


#define ERROR_CODE_OUT_OF_MEMORY    1000

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
} SErrorInfo, *ErrorInfo;

#endif
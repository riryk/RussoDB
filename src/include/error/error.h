
#ifndef ERROR_H
#define ERROR_H

#include "common.h"

#define ERROR_STACK_SIZE  5
#define TIME_STRING_LEN 128

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


#define ERROR_CODE_GENERAL                  1000
#define ERROR_CODE_OUT_OF_MEMORY            1001
#define ERROR_CODE_BLOCK_NOT_FOUND          1002
#define ERROR_CODE_INTERNAL_ERROR           1003
#define ERROR_CODE_WARNING                  1004
#define ERROR_CODE_SUCCESS                  1005
#define ERROR_CODE_BAD_FORMAT               1006
#define ERROR_CODE_INVALID_PARAM            1007
#define ERROR_CODE_MAX_ALLOC_MEM_EXCEEDED   1008
#define ERROR_CODE_CREATE_THREAD_FAILED     1009
#define ERROR_CODE_FILE_ACCESS              1010
#define ERROR_CODE_CREATE_PIPE_FAILED       1011
#define ERROR_CODE_CREATE_FILE_MAP_FAILED   1012
#define ERROR_CODE_MAP_MEMORY_TO_FILE       1013
#define ERROR_CODE_PROC_CMD_LINE_TO_LONG    1014
#define ERROR_CODE_CREATE_PROCESS_FAILED    1015
#define ERROR_CODE_DUPLICATE_HANDLE_FAILED  1016
#define ERROR_CODE_TERMINATE_PROCESS_FAILED 1017
#define ERROR_CODE_UNMAP_VIEW_OF_FILE       1018
#define ERROR_CODE_CLOSE_HANDLER_FAILED     1019
#define ERROR_CODE_RESERVE_MEMORY_FAILED    1020
#define ERROR_CODE_RESUME_THREAD_FAILED     1021
#define ERROR_CODE_REGISTER_WAIT_HANDLER_FAILED 1022
#define ERROR_CODE_START_SUB_PROC_FAILED    1023
#define ERROR_CODE_STDERR_REDIRECT_FAILED   1024
#define ERROR_CODE_DIR_CREATE_FAILED        1025
#define ERROR_CODE_FILE_OPEN_MAPPING_FAILED 1026
#define ERROR_CODE_LOOKUP_PRIVILEGE_FAILED  1027
#define ERROR_CODE_ADJUST_PRIVILEGE_FAILED  1028 
#define ERROR_CODE_TOCKEN_NOT_ENOUGH_PRIVILEDGES 1029
#define ERROR_CODE_OPEN_PROCESS_TOKEN       1030
#define ERROR_CODE_SET_PRIVILEDGE_FAILED    1031
#define ERROR_CODE_SPIN_LOCK_TIMEOUT        1032
#define ERROR_CODE_TYPE_OVERFLOW            1033
#define ERROR_CODE_TOO_MANY_LOCKS_TAKEN     1034
#define ERROR_CODE_BACKEND_PROC_NULL        1035
#define ERROR_CODE_CREATE_EVENT_FAILED      1036
#define ERROR_CODE_LOCK_SEMAPHORE_FAILED    1037
#define ERROR_CODE_TOO_MANY_SEMAPHORES      1038
#define ERROR_CODE_COULD_NOT_CREATE_SEMAPHORE 1039
#define ERROR_CODE_RELEASE_SEMAPHORE_FAILED 1040
#define ERROR_CODE_LOCK_NOT_HELD            1041
#define ERROR_CODE_CREATE_THREAD_FAILED     1042
#define ERROR_CODE_WAIT_FOR_SINGLE_OBJECT_FAILED 1043
#define ERROR_CODE_UNMAP_VIEW_OF_FILE_FAILED 1044
#define ERROR_CODE_VIRTUAL_FREE_FAILED      1045
#define ERROR_CODE_REATTACH_MEMORY_FAILED   1046
#define ERROR_CODE_RESERVE_MEMORY_FAILED    1047
#define ERROR_CODE_WAIT_FOR_TIMEOUT         1048
#define ERROR_CODE_TRY_SEMAPHORE_LOCK_FAILED 1049
#define ERROR_CODE_SET_CONSOLE_CTRL_HANDLER 1050

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


typedef struct SErrorCallback
{
	struct SErrorCallback*    prev;
	void   (*callback) (void* arg);
	void*   arg;
} SErrorCallback, *ErrorCallback;


#define ASSERT(logger, condition, retval) \
	if (!(condition)) \
    { \
	   (logger)->assert((condition)); \
	   return (retval); \
    }

#define ASSERT_VOID(logger, condition) \
	if (!(condition)) \
    { \
	   (logger)->assert((condition)); \
	   return; \
    }

#define ASSERT_ARG_VOID(logger, condition) \
	if (!(condition)) \
    { \
	   (logger)->assertArg((condition)); \
	   return; \
    }

#define ASSERT_ARG(logger, condition, retval) \
	if (!(condition)) \
    { \
	   (logger)->assertArg((condition)); \
	   return (retval); \
    }

#endif
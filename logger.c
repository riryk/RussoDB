#include "logger.h"
#include "stdio.h"

FILE* logFile = NULL;

/* Writes a message directly to an opened log file */
void write_message_file(
	char*               buffer, 
	int                 count)
{
	int			result;

	result = fwrite(buffer, 1, count, logFile);

	/* We have not written the message. 
	 * We should report an error.
	 */
	if (result != count)
		; // Should report an error
}

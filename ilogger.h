
#ifndef ILOGGER_H
#define ILOGGER_H

typedef struct SILogger
{
	void (*write_message_file)(
	     char*               buffer, 
	     int                 count);

} SILogger, *ILogger;

#endif
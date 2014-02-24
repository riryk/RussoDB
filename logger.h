#include "common.h"
#include "stdio.h"

#define PIPE_CHUNK_SIZE  512

typedef struct SPipeChunkHeader
{
	char		nuls[2];		/* always \0\0 */
	uint16		len;			/* chunk's size */
	int		    pid;			/* process id */
	char		isLast;		    /* last chunk of message? */
	char		data[1];		/* data */
} SPipeChunkHeader, *PipeChunkHeader;

typedef union
{
	SPipeChunkHeader  header;
	char		      filler[PIPE_CHUNK_SIZE];
} UPipeProtoChunk;


#define PIPE_CHUNK_HEADER_SIZE  offsetof(SPipeChunkHeader, data)
#define PIPE_CHUNK_MAX_LOAD  ((int)(PIPE_CHUNK_SIZE - PIPE_CHUNK_HEADER_SIZE))

extern FILE* logFile = NULL;

void write_message_file(
	char*               buffer, 
	int                 count);
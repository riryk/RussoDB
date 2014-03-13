#include "common.h"
#include "stdio.h"
#include "stddef.h"
#include "list.h"
#include "ilogger.h"

#define PIPE_CHUNK_SIZE  512
#define LOG_DESTINATION_STDERR 1

/*
 * We read() into a temp buffer twice as big as a chunk, so that any fragment
 * left after processing can be moved down to the front and we'll still have
 * room to read a full chunk.
 */
#define READ_BUF_SIZE (2 * PIPE_CHUNK_SIZE)

typedef struct SPipeChunkHeader
{
	char		nuls[2];		/* always \0\0 */
	int  		len;			/* chunk's size */
	int		    pid;			/* process id */
	char		isLast;		    /* last chunk of message? */
	char		data[1];		/* data */
} SPipeChunkHeader, *PipeChunkHeader;

typedef union
{
	SPipeChunkHeader  header;
	char		      filler[PIPE_CHUNK_SIZE];
} UPipeProtoChunk;

typedef struct SBuffer
{
	int		        proc_id;  /* identifier of source process */
	SStringInfo     data;     /* accumulated data, as a StringInfo */
} SBuffer, *Buffer;

#define PIPE_CHUNK_HEADER_SIZE  offsetof(SPipeChunkHeader, data)
#define PIPE_CHUNK_MAX_LOAD  ((int)(PIPE_CHUNK_SIZE - PIPE_CHUNK_HEADER_SIZE))
#define BUFFER_LISTS_COUNT 256

extern FILE* logFile;
List* buffer_lists[BUFFER_LISTS_COUNT];

void write_message_file(
	char*               buffer, 
	int                 count);

uint __stdcall pipeThread(void *arg);
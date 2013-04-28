
#include <stdio.h>
#include <stdarg.h>

#define ERR_OUT_OF_MEMORY                           100
#define ERR_INSUFFICIENT_FILE_DESCRIPTORS           101
#define ERR_CANNOT_OPEN_FILE                        102
#define ERR_TOO_MANY_OPEN_FILES                     103

#define ASSERT_NOT_ALLOWED_FILE_FLAGS               1000

void Fatal(long Code, char* Message,...);
void Log(long Code, char* Message,...);
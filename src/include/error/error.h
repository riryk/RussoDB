
#include <stdio.h>
#include <stdarg.h>

#define ERR_OUT_OF_MEMORY                           100
#define ERR_INSUFFICIENT_FILE_DESCRIPTORS           101
#define ERR_CANNOT_OPEN_FILE                        102
#define ERR_TOO_MANY_OPEN_FILES                     103
#define ERR_COULD_NOT_CREATE_TEMP_FILE              104
#define ERR_INSUFFICIENT_SYSTEM_RESOURCE            105
#define ERR_TEMP_FILES_LIMIT_EXCEEDED               106

#define ASSERT_NOT_ALLOWED_FILE_FLAGS               1000

void Fatal(long Code, char* Message,...);
void Log(long Code, char* Message,...);
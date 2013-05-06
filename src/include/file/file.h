
#include <stdio.h>
#include <io.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "error.h"
#include "common.h"
#include "osfile.h"

typedef unsigned long long int uint64;

void Init();
void DetermineMaxAllowedFileDescriptors();
int ROpenFile(char* FileName, int FileFlags, int FileMode);
int OpenTempFileInTablespace(int tableSpaceId, int error);


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "error.h"
#include "common.h"
#include "osfile.h"

typedef unsigned long long int uint64;

void Init();
void DetermineMaxAllowedFileDescriptors();
int FileOpen(char* FileName, int FileFlags, int FileMode);

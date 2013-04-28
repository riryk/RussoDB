
#include <stdio.h>
#include <errno.h>
#include "error.h"
#include "common.h"
#include "osfile.h"

void Init();
void DetermineMaxAllowedFileDescriptors();
int FileOpen(char* FileName, int FileFlags, int FileMode);

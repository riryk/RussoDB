
#include <windows.h>
#include <fcntl.h>
#include "error.h"

#define		O_FILE_DIRECT	0x80000000

int openFileBase(char *FileName, int FileFlags,...);
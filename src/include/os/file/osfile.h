
#include <windows.h>
#include <fcntl.h>
#include "error.h"
#include <stdio.h>

#define		O_FILE_DIRECT	0x80000000
#define     FILE_PATH_NOT_FOUND 42

int openFileBase(char *FileName, int FileFlags,...);

#include "common.h"

#ifndef FILE_MANAGER_HELPER_H
#define FILE_MANAGER_HELPER_H

extern char*   fnames[3000];
extern int     fds[3000];
extern void*   mem;
extern int     fcount;

void doCreateTestFiles(int filesCount);
void doFreeTestFiles();
void doFreeNotClosedTestFiles(int* alreadyClosed, int count);

#endif
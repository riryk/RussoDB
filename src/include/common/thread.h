#include "common.h"

#ifndef THREAD_H
#define THREAD_H

#ifdef _WIN32

typedef DWORD WINAPI (*ThreadFunc)(LPVOID);
typedef LPDWORD TThreadId; 
typedef HANDLE TThread;

#endif

#endif
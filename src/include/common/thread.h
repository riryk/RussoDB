#include "common.h"

#ifndef THREAD_H
#define THREAD_H

#ifdef _WIN32

typedef DWORD (WINAPI *threadFunc)(LPVOID param);
typedef void (*sleepFunc)(int milliseconds);

#define THREAD_FUNC LPTHREAD_START_ROUTINE

typedef LPDWORD TThreadId; 
typedef HANDLE TThread;
typedef HANDLE TEvent;

#endif

#endif
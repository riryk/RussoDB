
#include "common.h"

#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#define SIGNAL_COUNT 32

#define SIGNAL_IGNORE (void (__cdecl *)(int))1          

extern HANDLE  signalEvent;

typedef void (*signalFunc)(int);

#endif
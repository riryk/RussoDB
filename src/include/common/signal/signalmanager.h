
#include "common.h"
#include "isignalmanager.h"
#include "signal.h"

#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

extern HANDLE       signalEvent;
extern volatile int signalQueue;
extern int	        signalMask;

extern const SISignalManager sSignalManager;
extern const ISignalManager  signalManager;

void signalCtor(void* self);
void signalDtor(void* self);
void dispatchQueuedSignals();
void queueSignal(int signum);

DWORD __stdcall signalThreadFunc(LPVOID param);

BOOL WINAPI consoleHandler(
    void*          self, 
	DWORD          ctrlType);

signalFunc setSignal(
    void*          self,
    int            signum, 
	signalFunc     handler);

void queueSignal(int signum);
void dispatchQueuedSignals();

#endif
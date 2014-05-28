
#include "common.h"
#include "isignalmanager.h"

#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#define SIGNAL_COUNT 32

#define SIGNAL_DEFAULT (void (__cdecl *)(int))0       /* default signal action */
#define SIGNAL_IGNORE (void (__cdecl *)(int))1        /* ignore signal action */      
#define SIGNAL_GET (void (__cdecl *)(int))2           /* return current value */
#define SIGNAL_ERROR (void (__cdecl *)(int))3         /* signal error */
#define SIGNAL_ACKNOWLEDGE (void (__cdecl *)(int))4   /* acknowledge */

#define QUEUE_LEFT (signalQueue & ~signalMask)
#define SIGNAL_MASK(signal) (1 << ((signal)-1))

extern HANDLE  signalEvent;
extern volatile int signalQueue;
extern int	   signalMask;

extern const SISignalManager sSignalManager;
extern const ISignalManager  signalManager;

typedef void (*signalFunc)(int);

void signalCtor(void* self);
void dispatchQueuedSignals();

#endif
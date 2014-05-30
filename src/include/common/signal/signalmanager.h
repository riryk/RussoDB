
#include "common.h"
#include "isignalmanager.h"

#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

/* Signal types */
#define SIGNAL_INTERRUPT             2       /* signal interrupt */
#define SIGNAL_ILLEGAL               4       /* illegal instruction - invalid function image */
#define SIGNAL_FLOAT_POINT_EXC       8       /* floating point exception */
#define SIGNAL_SEGM_VIOLATION        11      /* segment violation */
#define SIGNAL_TERMINATION           15      /* software termination signal from kill */
#define SIGNAL_CTRL_BREAK            21      /* Ctrl-Break sequence */
#define SIGNAL_ABNORMAL_TERMINATION  22      /* abnormal termination triggered by abort call */

#define SIGNAL_COUNT 32

#define SIGNAL_DEFAULT (void (__cdecl *)(int))0        /* default signal action */
#define SIGNAL_IGNORE (void (__cdecl *)(int))1         /* ignore signal action */      
#define SIGNAL_GET (void (__cdecl *)(int))2            /* return current value */
#define SIGNAL_GET_ERROR (void (__cdecl *)(int))3      /* signal gets error */
#define SIGNAL_ACKNOWLEDGE (void (__cdecl *)(int))4    /* acknowledge */
#define SIGNAL_ERROR (void (__cdecl *)(int))-1         /* acknowledge error */

#define QUEUE_LEFT (signalQueue & ~signalMask)
#define SIGNAL_MASK(signal) (1 << ((signal)-1))

extern HANDLE       signalEvent;
extern volatile int signalQueue;
extern int	        signalMask;

extern const SISignalManager sSignalManager;
extern const ISignalManager  signalManager;

typedef void (*signalFunc)(int);

void signalCtor(void* self);
void dispatchQueuedSignals();

#endif
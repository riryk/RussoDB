
#ifndef SIGNAL_H
#define SIGNAL_H

/* Signal types */
#define SIGNAL_INTERRUPT             2       /* signal interrupt */
#define SIGNAL_ILLEGAL               4       /* illegal instruction - invalid function image */
#define SIGNAL_FLOAT_POINT_EXC       8       /* floating point exception */
#define SIGNAL_SEGM_VIOLATION        11      /* segment violation */
#define SIGNAL_TERMINATION           15      /* software termination signal from kill */
#define SIGNAL_CTRL_BREAK            21      /* Ctrl-Break sequence */
#define SIGNAL_ABNORMAL_TERMINATION  22      /* abnormal termination triggered by abort call */


/* Some extra signal types */
#define SIGNAL_HUP				     1
#define SIGNAL_QUIT				     3
#define SIGNAL_TRAP				     5
#define SIGNAL_KILL				     9
#define SIGNAL_PIPE				     13
#define SIGNAL_ALRM				     14
#define SIGNAL_STOP				     17
#define SIGNAL_TSTP				     18
#define SIGNAL_CONT				     19
#define SIGNAL_CHLD				     20
#define SIGNAL_TTIN				     21
#define SIGNAL_ABRT				     22	     
#define SIGNAL_WINCH			     28
#define SIGNAL_USR1				     30
#define SIGNAL_USR2				     31

#define SIGNAL_COUNT 32
#define MAX_CREATE_NAMED_PIPES_ATTEMPTS 20

#define SIGNAL_DEFAULT (void (__cdecl *)(int))0        /* default signal action */
#define SIGNAL_IGNORE (void (__cdecl *)(int))1         /* ignore signal action */      
#define SIGNAL_GET (void (__cdecl *)(int))2            /* return current value */
#define SIGNAL_GET_ERROR (void (__cdecl *)(int))3      /* signal gets error */
#define SIGNAL_ACKNOWLEDGE (void (__cdecl *)(int))4    /* acknowledge */
#define SIGNAL_ERROR (void (__cdecl *)(int))-1         /* acknowledge error */

#define QUEUE_LEFT() (signalQueue & ~signalMask)
#define SIGNAL_MASK(signal) (1 << ((signal)-1))

#define UNBLOCKED_SIGNAL_QUEUE()	(pg_signal_queue & ~pg_signal_mask)

#define PROCESS_INTERRUPTS(signMan) \
	do { \
	    if (QUEUE_LEFT()) \
           (signMan)->dispatchQueuedSignals(); \
	} while (0)

typedef void (*signalFunc)(int);
 
#endif
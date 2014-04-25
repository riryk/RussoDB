
#include "signalmanager.h"

#ifdef _WIN32

HANDLE	          signalEvent;
CRITICAL_SECTION  signalCritSec;

signalFunc        signalArray[SIGNAL_COUNT];
signalFunc        signalDefaults[SIGNAL_COUNT];

volatile int      signalQueue;
int			      signalMask;

void signalCtor()
{
    int			i;
	HANDLE		signalThread;

	InitializeCriticalSection(&signalCritSec);

	for (i = 0; i < SIGNAL_COUNT; i++)
	{
		signalArray[i]    = SIGNAL_IGNORE;
		signalDefaults[i] = SIGNAL_IGNORE;
	}
    
	signalQueue = 0;
    signalMask  = 0;
    
	/* Create the global event handle used to flag signals */
	signalEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (pgwin32_signal_event == NULL)
		ereport(FATAL,
				(errmsg_internal("could not create signal event: error code %lu", GetLastError())));
}

#endif
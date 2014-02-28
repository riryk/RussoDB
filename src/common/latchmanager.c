
#include "latchmanager.h"

const SILatchManager sLatchManager = { initLatch, setLatch };
const ILatchManager  latchManager  = &sLatchManager;

#ifdef _WIN32

void initLatch(void* self, Latch latch)
{
	ILatchManager  _    = (ILatchManager)self;
	IErrorLogger   elog = _->errorLogger;

    SECURITY_ATTRIBUTES   sa;

	latch->isset   = False;
	latch->ownerid = 0;
	latch->shared  = True;

    memset(&sa, 0, sizeof(sa)); 
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = True;

	latch->event = CreateEvent(&sa, TRUE, FALSE, NULL);
	if (latch->event == NULL)
		elog->log(LOG_ERROR, 
		          ERROR_CODE_GENERAL, 
				  "CreateEvent failed: error code %lu", 
				  GetLastError());
}

/* Sets a latch into active state. That means 
 * that processes or threads, which have been waiting for  
 * the latch, receive a notification that a latch 
 * has become active.
 */
void setLatch(Latch latch)
{
   	HANDLE		handle;

	/* If the latch is already in active state,
	 * we do not do anything and just return.
	 */
	if (latch->isset)
		return;

	/* Set a latch to True. */
	latch->isset = True;
	handle       = latch->event;

	/* If there is a windows event,
	 * we set it and thus notify all
	 * processes or threads waiting for it.
	 */
	if (handle != NULL)
		SetEvent(handle);
}

#endif
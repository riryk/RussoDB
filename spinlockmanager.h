
#include "ispinlockmanager.h"

int spinLockAcquire(
	  void*             self,
	  volatile long*    lock, 
	  char*             file, 
	  int               line);
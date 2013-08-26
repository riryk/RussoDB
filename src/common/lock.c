
#include "lock.h"
#include <windows.h>
#include <stdlib.h>

#define SLEEPS_MAX_COUNT	1000
#define MIN_SLEEP_MSEC      1   
#define SPINS_MAX_NUM       1000
#define SPINS_DEFAULT_NUM   100
#define SPINS_MIN_NUM       10
#define MAX_RANDOM_VALUE    (0x7FFFFFFF)
#define SLEEP_MAX           1000

static int spinsAllowedCount = SPINS_DEFAULT_NUM;

#define S_UNLOCK(lock)		(*((volatile slock_t *) (lock)) = 0)

void SpinLockRelease(volatile long* lock)
{
   *lock = 0;
}

int SpinLockAcquire(volatile long* lock, char* file, int line)
{
    int			spinCount = 0;
	int			sleepsCount = 0;
	int			sleep = 0;

	/* This function compares lock variable with
	 * the latest parameter 0 and if it is equal to 0
	 * it sets to 1.
	 * Not thread safe analog
	 *
	 * if (lock == 0)
	 *    lock = 1;
	 * 
	 * Assembler code for this:
	 *  MOV EAX, [lock]        // read lock variable from RAM into register EAX
	 *  CMP EAX, 0             // compare EAX with 0
	 *  JZ set_1
	 *  set_1: 
	 *  ADD EAX 1
	 * 
	 * We need explanation for what will happen if we 
	 * Bad example
	 * while (lock ) 
	 * From the start lock = 0;
	 * And suppose two threads reads 0 into their registers
	 * They both compare it to 0 and set it to 1 and skip the while loop
	 * 
	 * After skipping the while loop they both have access to unique resources
	 * which is unacceptable. Only one thread can have access.
	 * InterlockedCompareExchange can make operations atomic: 
	 * the first thread read 0 into register. The second thread is blocked.
	 * Then the first thread set it to 1. After that the second thread reads it
	 * and goes to while loop
	 */ 
    while (InterlockedCompareExchange(lock, 1, 0))
	{
        /* CPU-specific delay each time through the loop */

		/* Explanation from documentation
		 * Improves the performance of spin-wait loops. 
		 * When executing a “spin-wait loop,” a Pentium 4 
		 * or Intel Xeon processor suffers a severe performance penalty 
		 * when exiting the loop because it detects a possible memory order violation. 
		 * The PAUSE instruction provides a hint to the processor 
		 * that the code sequence is a spin-wait loop. 
		 * The processor uses this hint to avoid the memory order violation in most situations, 
		 * which greatly improves processor performance. 
		 * For this reason, it is recommended that a PAUSE instruction be placed in all spin-wait loops.
		 */
		__asm rep nop;

		/* From the start we do some count of simple spins before 
		 * suspending the thread. It can be very efficient when contention time is small 
		 * In single processor machine while one thread is spinning, the other 
		 * thread is waiting. Here works delaying
		 */
		if (++spinCount >= spinsAllowedCount)
		{
           /* If we have exceeded the maximum allowed spin count
		    * it would be better to suspend the thread. */ 
           if (++sleepsCount >= SLEEPS_MAX_COUNT)
		   {
			   /* In this case spin lock has been waiting for a long time
			    * so we need to throw a timeout exception */
			   Log("Spillock exceeded allowed timeout: %d", SLEEPS_MAX_COUNT);
		   }

		   if (sleep == 0) /* first time to delay? */
				sleep = MIN_SLEEP_MSEC;
           
		   Sleep(sleep * 1000L);

		   /* How we increase the sleep timeout? 
		    * we increase it from [1.5 to 2] */
		   sleep += (int) (sleep *
					  ((double)rand() / (double)MAX_RANDOM_VALUE) + 0.5);

		   if (sleep > SLEEP_MAX)
			   sleep = SLEEP_MAX;

		   //spins = 0;
		}
	}
    /* If sleeps count is 0, we can say that we are on a multiprocessor machine
	 * so we need to increase spin lock count as much as possible
	 * If sleeps count is not 0, we suppose that we are on single processor
	 * machine and we need to reduce spin lock count as min as possible */
	if (sleep == 0)
	{
		/* we never had to delay */
		if (spinsAllowedCount < SPINS_MAX_NUM)
			spinsAllowedCount = 
			    (spinsAllowedCount + 100 < SPINS_MAX_NUM) 
				? spinsAllowedCount + 100
				: SPINS_MAX_NUM;
	}
	else
	{
		if (spinsAllowedCount > SPINS_MIN_NUM)
            spinsAllowedCount = 
			    (spinsAllowedCount - 1 > SPINS_MIN_NUM)
				? spinsAllowedCount - 1
				: SPINS_MIN_NUM;
	}
}

#include "lock.h"

#define MIN_SPINS_PER_DELAY 10
#define MAX_SPINS_PER_DELAY 1000
#define DELAYS_MAX_COUNT	1000
#define MIN_DELAY_MSEC		1
#define MAX_DELAY_MSEC		1000

#define SPINS_MAX_NUM       100


int SpinLock(volatile long* lock, char* file, int line)
{
    int			spinCount = 0;
	int			delaysCount = 0;
	int			cur_delay = 0;

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
		 * suspending the thread. It can be very efficient when contention time is small */
		if (++spinCount >= SPINS_MAX_NUM)
		{
           /* If we have exceeded the maximum allowed spin count
		    * it would be better to suspend the thread. */ 
           if (++delaysCount >= DELAYS_MAX_COUNT)
		   {

		   }
		}
	}
}
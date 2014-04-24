
#include "spinlockmanager.h"

#define SPINS_DEFAULT_NUM   100
#define SLEEPS_MAX_COUNT	1000
#define SLEEP_MAX           1000
#define MAX_RANDOM_VALUE    (0x7FFFFFFF)

int spinsAllowedCount = SPINS_DEFAULT_NUM;

int spinLockAcquire(
	void*             self,
	volatile long*    lock, 
	char*             file, 
	int               line)
{
	ISpinLockManager slm  = (ISpinLockManager)self;
	IErrorLogger     elog = slm->errorLogger;

    int			spinCount = 0;
	int			sleepsCount = 0;
	int			sleep = 0;

	/* This function compares lock variable with
	 * the latest parameter 0 and if it is equal to 0 it is set to 1.
	 * 
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
	 * And suppose two threads read 0 into their registers
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
        /* CPU-specific delay each time through the loop 
		 * Explanation from documentation
		 * Improves the performance of spin-wait loops. 
		 * When executing a “spin-wait loop,” a Pentium 4 
		 * or Intel Xeon processor suffers a severe performance penalty 
		 * when exiting the loop because it detects a possible memory order violation. 
		 * The PAUSE instruction provides a hint to the processor 
		 * that the code sequence is a spin-wait loop. 
		 * The processor uses this hint to avoid the memory order violation in most situations, 
		 * which greatly improves processor performance. 
		 * For this reason, it is recommended that 
		 * a PAUSE instruction be placed in all spin-wait loops.
		 */
		__asm rep nop;

		/* From the start we do some count of simple spins before 
		 * suspending the thread. It can be very efficient 
		 * when contention time is small 
		 * In single processor machine while one thread is spinning, 
		 * the other thread is waiting. Here works delaying.
		 */
		if (++spinCount >= spinsAllowedCount)
		{
           /* If we have exceeded the maximum allowed spin count
		    * it would be better to suspend the thread. 
			*/ 
           if (++sleepsCount >= SLEEPS_MAX_COUNT)
		   {
			   /* In this case spin lock has been waiting for a long time
			    * so we need to throw a timeout exception.
				* We have exceeded the maximum number of allowed sleep counts.
				* The thread is stuck. So we should report a panic error.
				*/
			   elog->log(LOG_PANIC,
			      ERROR_CODE_SPIN_LOCK_TIMEOUT,
				  "Spinlock exceeded max allowed sleep counts: %d",
                  SLEEPS_MAX_COUNT);
		   }

		   /* When sleep == 0 it means that we are intending to 
		    * sleep for the first time. So that we set sleep to 
			* minimum sleep milliseconds.
		    */
		   if (sleep == 0)
			   sleep = MIN_SLEEP_MSEC;

		   /* Sleep for sleep milliseconds. */
		   Sleep(sleep * 1000L);

		   /* How we increase the sleep timeout? 
		    * we increase it from [1.5 to 2] */
		   sleep += (int) (sleep *
					  ((double)rand() / (double)MAX_RANDOM_VALUE) + 0.5);

		   /* We have exceeded maximum sleep count. 
		    * Return it to the maximum allowed 
		    */
		   if (sleep > SLEEP_MAX)
			   sleep = SLEEP_MAX;

		   /* Reset spin count and continue spinning once more. */
		   spinCount = 0;
		}
	}
    /* If sleeps count is 0, we can say that we are on a multiprocessor machine.
	 * If sleeps count is not 0, we are on uniprocessor machine.
	 * This is only an assumption. Due to context switching on 
	 * single-processor machine it is most probable that wehave to wait.
	 * In case of a uniprocessor machine we need to deccrease 
	 * spin lock count as much as possible. 
	 * (Because spinning on a uniprocessor machine 
	 *  is a complete waste of time.)
	 */
	if (sleep == 0)
	{
		/* In this case we have not slept at all. 
		 * So there is suspision that we are on a 
		 * multiprocessor environment. So that we 
		 * need to have spinsAllowedCount as much as 
		 * possible. So that if we have not exceeded 
		 * the maximum allowed spins count we increase 
		 * spinsAllowedCount for 100 points.
		 */
		if (spinsAllowedCount < SPINS_MAX_NUM)
			spinsAllowedCount = Min(spinsAllowedCount + 100, SPINS_MAX_NUM);

		return sleepsCount;
	}
	
	/* Here sleep count != 0. Probably we are on uniprocessor
	 * machine. In this case we need to have spins allowed count
	 * as small as possible. And we decrease spins allowed count 
	 * for 1 point if we do not exceed spins min count.
	 */
	if (spinsAllowedCount > SPINS_MIN_NUM)
        spinsAllowedCount = Max(spinsAllowedCount - 1, SPINS_MIN_NUM);

	return sleepsCount;
}

void spinLockRelease(volatile long* lock)
{
    *lock = 0;
}

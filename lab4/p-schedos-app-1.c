#include "process.h"
#include "x86sync.h"

/*****************************************************************************
 * p-schedos-app-1
 *
 *   This tiny application prints red "1"s to the console.
 *   It yields the CPU to the kernel after each "1" using the sys_yield()
 *   system call.  This lets the kernel (kernel.c) pick another
 *   application to run, if it wants.
 *
 *   The other p-schedos-app-* processes simply #include this file after defining
 *   PRINTCHAR appropriately.
 *
 *****************************************************************************/

#ifndef PRINTCHAR
#define PRINTCHAR	('1' | 0x0C00)
#endif

void
pmain(void)
{
	int i;
	//sys_prior(6);
	for (i = 0; i < RUNCOUNT; i++) {
		
		sys_lock_acquire();
		// Write characters to the console, yielding after each one.
		
		*cursorpos++ = PRINTCHAR;
		sys_lock_release();
		sys_yield();
	}

	sys_exit(0);

	/* this is the original code of the lab
	Yield forever.
	while (1)
		sys_yield();
	*/
	
}

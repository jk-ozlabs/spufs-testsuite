#define BUFFER_SIZE	4096
#define BUFFER_BYTES	(BUFFER_SIZE * sizeof(int))
/*#define NUM_ITERS	1000000*/
#define NUM_ITERS	1000000
#define BUFFER_SIGNATURE	0xb00bfeed

#include <stdio.h>
#define DBG(x...)	printf(ROLE ": " x)

#define NANOS_PER_SECOND (1.0E9)

#define CPU_TIME_BASE (14318000.0) /* Clocks per second */

inline double decr_to_bw(unsigned int ticks)
{
	double secs, nanos, usecs;

	secs = ticks / CPU_TIME_BASE;
	nanos = (secs - (unsigned int)secs) * NANOS_PER_SECOND;

	usecs = secs * 1.0E6 + nanos / 1.0E3;

	return (BUFFER_BYTES / usecs) * NUM_ITERS;
}

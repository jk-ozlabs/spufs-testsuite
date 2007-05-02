#define NANOS_PER_SECOND (1.0E9)

#define CPU_TIME_BASE (14318000.0) /* Clocks per second */

static double decr_to_usecs(unsigned int start, unsigned int end)
{
	double secs, nanos;

	secs = ((start - end) / CPU_TIME_BASE);
	nanos = (secs - (unsigned int)secs) * NANOS_PER_SECOND;

	return secs * 1.0E6 + nanos / 1.0E3;
}

static double usecs_to_bw(double usecs)
{
	return (BUFFER_BYTES / usecs) * N_ITERS;
}

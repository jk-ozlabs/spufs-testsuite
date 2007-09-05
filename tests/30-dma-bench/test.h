/*
 * Testsuite for the Linux SPU filesystem
 *
 * Copyright (C) IBM Corporation, 2007
 *
 * Author: Jeremy Kerr <jk@ozlabs.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
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

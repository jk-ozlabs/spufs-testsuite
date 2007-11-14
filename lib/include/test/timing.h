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
#ifndef _TIMING_H
#define _TIMING_H

#include <sys/time.h>
#include <time.h>
#include <stdio.h>


static inline void timer_start(struct timeval *time)
{
	gettimeofday(time, NULL);
}

static inline void timer_stop_and_print(struct timeval *start_time)
{
	struct timeval end_time;

	gettimeofday(&end_time, NULL);

	if (end_time.tv_usec < start_time->tv_usec) {
		end_time.tv_usec += 1000000;
		end_time.tv_sec -= 1;
	}

	printf("%ld.%03ld\n", end_time.tv_sec - start_time->tv_sec,
			(end_time.tv_usec - start_time->tv_usec) / 1000);
}

#endif /* _TIMING_H */

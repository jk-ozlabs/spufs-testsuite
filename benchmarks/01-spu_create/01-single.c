
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
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <test/spu_syscalls.h>
#include <test/timing.h>

#define ITERATIONS 16384

int main(void)
{
	int rc, i;
	const char *name = "/spu/01-single";
	struct timeval tv;

	timer_start(&tv);

	for (i = 0; i < ITERATIONS; i++) {
		rc = spu_create(name, 0, 0);
		assert(rc >= 0);
		close(rc);
	}

	timer_stop_and_print(&tv);

	return EXIT_SUCCESS;
}

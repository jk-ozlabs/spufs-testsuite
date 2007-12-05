/*
 * Testsuite for the Linux SPU filesystem
 *
 * Copyright (C) IBM Corporation, 2007
 *
 * Author: Michael Ellerman <michael@ellerman.id.au>
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
#define _ATFILE_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include <test/spu_syscalls.h>

int main(void)
{
	int i, ctx[16];
	char name[256];

	for (i = 0; i < 16; i++) {
		snprintf(name, 256, "/spu/coredump-%d-%d", getpid(), i);
		ctx[i] = spu_create(name, 0, 0755);
		assert(ctx[i] >= 0);
	}

	printf("%d\n", getpid());
	fflush(stdout);

	*(int *)NULL = 1;

	return EXIT_SUCCESS;
}

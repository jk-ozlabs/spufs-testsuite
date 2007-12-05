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
#include <test/utils.h>
#include <test/spu_syscalls.h>
#include <stdlib.h>
#include <errno.h>

const unsigned long spu_create_all_flags =
	SPU_CREATE_EVENTS_ENABLED |
	SPU_CREATE_GANG |
	SPU_CREATE_NOSCHED |
	SPU_CREATE_ISOLATE |
	SPU_CREATE_AFFINITY_SPU |
	SPU_CREATE_AFFINITY_MEM |
	0;

int main(void)
{
	char *name = name_spu_context(NULL);
	int bit, ctx;

	for (bit = 0; bit < 32; bit++) {
		unsigned long flags = 1ul << bit;
		if (flags & spu_create_all_flags)
			continue;
		ctx = spu_create(name, flags, 0);
		if (ctx != -1) {
			fprintf(stderr, "spu_create() accepted invalid flags: "
					"0x%08lx\n", flags);
			return EXIT_FAILURE;
		}
		if (errno != EINVAL) {
			fprintf(stderr, "spu_create(): wrong errno (%d) with "
					"invalid flags: 0x%08lx\n",
					errno, flags);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

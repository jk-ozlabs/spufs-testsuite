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
#include <test/spu_syscalls.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int main(void)
{
	int gang, ctx[2];
	const char *names[] = {"/spu/gang-01/",
			       "/spu/gang-01/02-affinity-create-spu.1",
			       "/spu/gang-01/02-affinity-create-spu.2"};

	gang = spu_create(names[0], SPU_CREATE_GANG, 0755);
	assert(gang >= 0);

	ctx[0] = spu_create(names[1], 0, 0);
	assert(ctx[0] >= 0);

	ctx[1] = spu_create(names[2], SPU_CREATE_AFFINITY_SPU, 0, ctx[0]);

	if (ctx[1] < 0) {
		perror("spu_create");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

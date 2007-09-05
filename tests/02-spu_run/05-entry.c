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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>

/*
 * test that the entry point advances by 4 for each spu_run, and that it wraps
 * once we reach the end of local store
 */

int main(void)
{
	int rc, ctx;
	const char *name = "/spu/05-entry";
	uint32_t entry;

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	entry = 0;
	do {
		int expected_entry = (entry + 4) % LS_SIZE;
		rc = spu_run(ctx, &entry, NULL);

		if (rc < 0) {
			perror("spu_run");
			return EXIT_FAILURE;
		}

		if (entry != expected_entry) {
			fprintf(stderr, "entry = 0x%x, expected 0x%x\n",
					entry, expected_entry);
			return EXIT_FAILURE;
		}
	} while (entry != 0);

	return EXIT_SUCCESS;
}

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

/**
 * Ensure that the npc of spu_run is used when we run a NOSCHED context
 */

#define _ATFILE_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>

#include <test/spu_syscalls.h>

#if 0
	stop 0x1336
_start:
	.globl _start
	stop 0x1337
#endif
static uint32_t spe_program[] = {
	0x00001336, /* stop                           */
/* _start: */
	0x00001337, /* stop                           */
};
static uint32_t spe_program_entry = 0x00000004;

int main(void)
{
	int ctx, rc, ls_fd;
	const char *name = "/spu/03-npc";
	uint32_t entry  = spe_program_entry;

	ctx = spu_create(name, SPU_CREATE_NOSCHED, 0755);
	assert(ctx >= 0);

	ls_fd = openat(ctx, "mem", O_WRONLY);
	assert(ls_fd >= 0);

	rc = write(ls_fd, spe_program, sizeof(spe_program));
	assert(rc == sizeof(spe_program));

	rc = spu_run(ctx, &entry, NULL);

	if (rc < 0) {
		perror("spu_run");
		return EXIT_FAILURE;
	}

	if (rc == 0x13360002) {
		fprintf(stderr, "nosched context was started from 0x0, "
				"not 0x%x\n", spe_program_entry);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

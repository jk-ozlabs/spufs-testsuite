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
#define _ATFILE_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>


/*
 * test to set all of local store - fill LS with instructions:
 *
 * br	2
 * stop 0x0
 * ...
 * ...
 * nop
 * stop 0x1337
 *
 * So the pc skips over every odd instruction, and we should see a 0x1337 stop
 * code.
 */

int main(void)
{
	int ctx, ls_fd, rc, i;
	const char *name = "/spu/02-ppe-setup-all";
	uint32_t buf[2], entry, status;

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	ls_fd = openat(ctx, "mem", O_RDWR);
	if (ls_fd < 0) {
		perror("openat");
		return EXIT_FAILURE;
	}

	/* setup the skip-odd-instructions for all but the last two slots */
	buf[0] = 0x32000100; /* br   2		*/
	buf[1] = 0x00000000; /* stop 0x0	*/
	for (i = 0; i < LS_SIZE / sizeof(buf) - 1; i++) {
		rc = write(ls_fd, &buf, sizeof(buf));
		if (rc != sizeof(buf)) {
			perror("write");
			return EXIT_FAILURE;
		}
	}

	/* add the final nop-and-stop at the end of LS */
	buf[0] = 0x40200000; /* nop		*/
	buf[1] = 0x00001337; /* stop 0x1337	*/
	rc = write(ls_fd, &buf, sizeof(buf));
	if (rc != sizeof(buf)) {
		perror("write");
		return EXIT_FAILURE;
	}

	entry = 0;
	rc = spu_run(ctx, &entry, &status);

	if (rc < 0) {
		perror("spu_run");
		return EXIT_FAILURE;
	}

	if (!(rc & 0x2)) {
		fprintf(stderr, "spu didn't stop-and-signal?\n");
		return EXIT_FAILURE;
	}

	if (rc >> 16 != 0x1337) {
		fprintf(stderr, "spu executed different stop instruction?\n");
		return EXIT_FAILURE;
	}

	printf("npc: 0x%x, status: 0x%x\n", entry, rc);

	return EXIT_SUCCESS;
}

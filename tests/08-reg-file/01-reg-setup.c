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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>

/**
 * Create a SPE program:
 *  li r0,0
 *  li r1,1
 *  li r2,2
 *   ...
 *  li r127,127
 *
 * So each register is loaded with its register number. Run the program, and
 * check for this pattern in the reg file
 */

const int nr_regs = 128;

/**
 * Given a register 'reg', create an instruction to load the register number
 * into that register
 */
static uint32_t load_immediate_insn(int reg)
{
	return 0x40800000 | (reg << 7) | reg;
}

int main()
{
	int ctx, ls_fd, reg_fd, rc, i;
	uint32_t *ls_map, entry;
	char *name = "/spu/01-reg-setup";

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	ls_fd = openat(ctx, "mem", O_RDWR);
	assert(ls_fd >= 0);

	ls_map = mmap(NULL, LS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			ls_fd, 0);
	assert(ls_map != MAP_FAILED);

	reg_fd = openat(ctx, "regs", O_RDONLY);
	if (reg_fd <= 0) {
		perror("open:regs");
		return EXIT_FAILURE;
	}

	for (i = 0; i < nr_regs; i++)
		ls_map[i] = load_immediate_insn(i);
	ls_map[i] = 0x00001337; /* stop 0x1337 */

	entry = 0;
	rc = spu_run(ctx, &entry, NULL);

	if (rc < 0) {
		perror("spu_run");
		return EXIT_FAILURE;
	}

	if (rc != 0x13370002) {
		fprintf(stderr, "Incorrect stop code 0x%08x?\n", rc);
		return EXIT_FAILURE;
	}

	for (i = 0; i < nr_regs; i++) {
		uint32_t reg[4];

		if (read(reg_fd, &reg, sizeof(reg)) != sizeof(reg)) {
			perror("read");
			return EXIT_FAILURE;
		}

		if (reg[0] != i) {
			fprintf(stderr, "Incorrect register value: "
					"0x%08x != 0x%08x\n", reg[0], i);
			return EXIT_FAILURE;
		}

		printf("[%3d]: 0x%08x 0x%08x 0x%08x 0x%08x\n",
				i, reg[0], reg[1], reg[2], reg[3]);
	}

	return EXIT_SUCCESS;

}

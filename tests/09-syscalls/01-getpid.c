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
#include <stdint.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>
#include <inttypes.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>

/* Set up a syscall block in the SPE's local store, and then run the SPE
 * app (just a stop-and-signal) to invoke the syscall from the SPE code.
 *
 * We use getpid, and compare the result with calling getpid() on the PPE.
 *
 * The syscall block is set up at ls address 0x8, to ensure we can calculate
 * its address properly.
 */

struct spe_syscall_block {
	uint64_t nr_ret;
	uint64_t parm[6];
};

#if 0
	. = 8 * 8
	stop	0x2104
	.long	0x08
	stop	0x1337
#endif
static uint32_t spe_program[] = {
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00002104, /* stop                           */
	0x00000008, /* stop                           */
	0x00001337, /* stop                           */
};

int main()
{
	int ctx, ls_fd, rc;
	struct spe_syscall_block *syscall_block;
	char *name = "/spu/01-getpid";
	uint32_t entry;
	void *ls_map;
	pid_t pid;

	ctx = spu_create(name, 0, 0755);

	ls_fd = openat(ctx, "mem", O_RDWR);
	assert(ls_fd >= 0);

	ls_map = mmap(NULL, LS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			ls_fd, 0);
	assert(ls_map != MAP_FAILED);

	memcpy(ls_map, spe_program, sizeof(spe_program));

	/* setup the syscall block at LS address 0x08 */
	syscall_block = ls_map + 0x08;
	syscall_block->nr_ret = __NR_getpid;

	/* the SPE program starts at 8 u64s from the start of LS */
	entry = 8 * 8;
	rc = spu_run(ctx, &entry, NULL);

	if (rc < 0) {
		perror("spu_run");
		return EXIT_FAILURE;
	}

	if (rc != 0x13370002) {
		fprintf(stderr, "spu_run returned unexepected exit code "
				"0x%x\n", rc);
		return EXIT_FAILURE;
	}

	pid = getpid();

	if (syscall_block->nr_ret != pid) {
		fprintf(stderr, "SPE getpid() returned %" PRIu64 ", but PPE "
				"getpid() returned %d\n",
				syscall_block->nr_ret, pid);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}




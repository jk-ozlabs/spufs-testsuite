/*
 * Testsuite for the Linux SPU filesystem
 *
 * Copyright (C) IBM Corporation, 2008
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
#include <assert.h>
#include <fcntl.h>

#include <test/spu_syscalls.h>
#include <test/timing.h>

#define ITERATIONS 1000000

#if 0
n_iters:
	.long 0x00000000
	.long 0x00000000
	.long 0x00000000
	.long 0x00000000
syscall_block:
	.long 0
	.long 20	/* NR_getpid */
	.space 7 * 8
.globl _start
_start:
	lqa	r0,n_iters
	il	r1,0
	lqa	r2,syscall_block
loop:
	stqa	r2,syscall_block	/* re-set the syscall block */
	stop	0x2104			/* perform syscall */
	.long	syscall_block		/* pointer to the syscall block */
	ai	r1,r1,1
	ceq	r3,r0,r1
	brz	r3,loop

	stop	0x1337
#endif
static uint32_t spe_program[] = {
/* n_iters: */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
/* syscall_block: */
	0x00000000, /* stop                           */
	0x00000014, /* stop                           */
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
/* _start: */
	0x30800000, /* lqa     r0,0                   */
	0x40800001, /* il      r1,0                   */
	0x30800202, /* lqa     r2,10 <syscall_block>  */
/* loop: */
	0x20800202, /* stqa    r2,10 <syscall_block>  */
	0x00002104, /* stop                           */
	0x00000010, /* stop                           */
	0x1c004081, /* ai      r1,r1,1                */
	0x78004003, /* ceq     r3,r0,r1               */
	0x207ffd83, /* brz     r3,5c <loop>    # 5c   */
	0x00001337, /* stop                           */
};
static uint32_t spe_program_entry = 0x00000050;

int main(void)
{
	int rc, ctx, ls_fd;
	const char *name = "/spu/01-syscalls";
	struct timeval tv;

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	ls_fd = openat(ctx, "mem", O_RDWR);
	assert(ls_fd >= 0);

	/* update n_iters */
	spe_program[0] = ITERATIONS;

	write(ls_fd, spe_program, sizeof(spe_program));

	timer_start(&tv);

	rc = spu_run(ctx, &spe_program_entry, NULL);
	if (rc != 0x13370002) {
		perror("spu_run");
		printf("rc: %x\n", rc);
		return EXIT_FAILURE;
	}

	timer_stop_and_print(&tv);

	return EXIT_SUCCESS;
}

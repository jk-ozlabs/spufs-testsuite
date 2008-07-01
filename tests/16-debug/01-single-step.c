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

/* Run a simple SPE program under single step mode, by using
 * ptrace(PTRACE_SINGLESTEP). We should see the spe program stop before we
 * hit the program's final stop-and-signal instruction
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>

#ifndef PTRACE_GETREGS
#define PTRACE_GETREGS 12
#endif

#ifndef ERESTARTSYS
#define ERESTARTSYS	512
#endif


/* single-step-mode doesn't necessarily step by one instruction; we may see
 * a group of instructions issued. So, test with a number of nops
 */
#if 0
nop
nop
nop
nop
stop 0x1337
#endif

static uint32_t spe_program[] = {
	0x40200000, /* nop     r0                     */
	0x40200000, /* nop     r0                     */
	0x40200000, /* nop     r0                     */
	0x40200000, /* nop     r0                     */
	0x00001337, /* stop                           */
};

int check_spu_run(int ctx)
{
	uint32_t entry = 0;
	int rc, stopped = 0;

	rc = ptrace(PTRACE_TRACEME, 0, NULL, NULL);
	assert(!rc);

	/* send a signal to ourselves, so that the parent can attach */
	raise(SIGTRAP);

	/* loop, expecting either a single-step stop, or the final
	 * stop-and-signal */
	for (;;) {
		errno = 0;
		rc = spu_run(ctx, &entry, NULL);
		printf("status 0x%08x, errno %3d entry: 0x%08x\n",
				rc, errno, entry);

		/* if spu_run returned -1, expect -ERESTARTSYS */
		if (rc == -1) {
			if (errno != ERESTARTSYS) {
				perror("spu_run");
				return -1;
			}
			stopped = 1;

		} else if (rc == 0x13370002) {
			break;

		} else {
			fprintf(stderr, "spu_run exited with unexpected "
					"0x%08x\n", rc);
			return -1;
		}
	}

	/* error if we haven't enounted any single-step stop condition */
	if (!stopped)
		return -1;

	return 0;
}

/*
 * ptrace(PTRACE_SYSCALL) the process specified by pid, until we get to
 * spu_run. During this syscall, ptrace(PTRACE_SINGLESTEP).
 */
int ptrace_singlestep_spu_run(int pid)
{
	int rc, status;
	struct pt_regs regs;

	waitpid(pid, &status, 0);

	for (;;) {
		/* run until the next syscall */
		rc = ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
		assert(!rc);
		waitpid(pid, &status, 0);

		if (WIFEXITED(status))
			break;

		/* get the process' registers, to check the syscall number */
		rc = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
		assert(!rc);

		if (regs.gpr[0] == SYS_spu_run) {
			/* we're in spu_run, so singlestep */
			rc = ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL);
			if (rc < 0) {
				perror("ptrace: SINGLESTEP");
				return -1;
			}
			waitpid(pid, &status, 0);
		}
	}

	return WEXITSTATUS(status);
}

int main(void)
{
	int ctx, ls_fd, pid;
	const char *name = "/spu/06-single-step";
	char *ls_map;

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	ls_fd = openat(ctx, "mem", O_RDWR);
	assert(ls_fd >= 0);

	ls_map = mmap(NULL, LS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			ls_fd, 0);
	assert(ls_map != MAP_FAILED);

	memcpy(ls_map, spe_program, sizeof(spe_program));

	pid = fork();
	assert(pid >= 0);

	if (pid == 0)
		return check_spu_run(ctx);
	else
		return ptrace_singlestep_spu_run(pid);
}

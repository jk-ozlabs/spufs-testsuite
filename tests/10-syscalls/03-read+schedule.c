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
#include <pthread.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>

#define NR_ITERATIONS 10240

/* Set up a syscall block in the SPE's local store, and then run the SPE
 * app (just a stop-and-signal) to invoke the syscall from the SPE code.
 *
 * Use read as the syscall, then check to see that the read has been
 * performed to the correct address.
 *
 * The syscall block is set up at ls address 0x8, to ensure we can calculate
 * its address properly.
 */

struct spe_syscall_block {
	uint64_t nr_ret;
	uint64_t parm[6];
};

struct spe_thread_info {
	pthread_t	pthread;
	int		ctx;
	int		run_rc;
	uint32_t	entry;
};

struct read_thread_info {
	pthread_t	pthread;
	int		fd;
	int		run;
};

static void *spe_thread_fn(void *data)
{
	struct spe_thread_info *thread = data;

	thread->run_rc = spu_run(thread->ctx, &thread->entry, NULL);
	if (thread->run_rc < 0)
		perror("spu_run");

	return NULL;
}

static void *read_thread_fn(void *data)
{
	struct read_thread_info *thread = data;
	char buf[16];
	int rc;

	while (thread->run) {
		rc = read(thread->fd, &buf, sizeof(buf));
		assert(rc == sizeof(buf));

		rc = lseek(thread->fd, 0, SEEK_SET);
		assert(rc == 0);
	}

	return NULL;
}

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
	int ls_fd, regs_fd, rc, i, pipe_fds[2];
	struct spe_syscall_block *syscall_block;
	struct read_thread_info read_thread;
	struct spe_thread_info spe_thread;
	char *name = "/spu/03-read+schedule";
	char in, out;
	void *ls_map;

	rc = pipe(pipe_fds);
	assert(!rc);

	spe_thread.ctx = spu_create(name, 0, 0755);

	ls_fd = openat(spe_thread.ctx, "mem", O_RDWR);
	assert(ls_fd >= 0);

	regs_fd = openat(spe_thread.ctx, "regs", O_RDWR);
	assert(regs_fd >= 0);

	ls_map = mmap(NULL, LS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			ls_fd, 0);
	assert(ls_map != MAP_FAILED);

	memcpy(ls_map, spe_program, sizeof(spe_program));

	/* setup the syscall block at LS address 0x08 */
	syscall_block = ls_map + 0x08;
	syscall_block->nr_ret = __NR_read;
	/* syscall: read(pipe_fds[0], &in, sizeof(in))) */
	syscall_block->parm[0] = pipe_fds[0];
	syscall_block->parm[1] = (uint64_t)(unsigned long)&in;
	syscall_block->parm[2] = sizeof(in);

	/* start our thread to keep descheduling the context */
	read_thread.run = 1;
	read_thread.fd = regs_fd;
	rc = pthread_create(&read_thread.pthread, NULL,
			read_thread_fn, &read_thread);
	assert(!rc);

	for (i = 0; i < NR_ITERATIONS; i++) {
		/* the SPE program starts at 8 u64s from the start of LS */
		spe_thread.entry = 8 * 8;

		/* this will be overwritten on each run, so re-set */
		syscall_block->nr_ret = __NR_read;

		/* ... and the spe thread */
		rc = pthread_create(&spe_thread.pthread, NULL,
				spe_thread_fn, &spe_thread);
		assert(!rc);

		out = 0x13;
		write(pipe_fds[1], &out, sizeof(out));

		rc = pthread_join(spe_thread.pthread, NULL);

		if (spe_thread.run_rc != 0x13370002) {
			fprintf(stderr, "spu_run returned unexepected exit "
					"code 0x%x\n", spe_thread.run_rc);
			return EXIT_FAILURE;
		}

		if (syscall_block->nr_ret != sizeof(in)) {
			fprintf(stderr, "SPE read() returned %" PRId64 ", "
					"expecting %d\n",
					syscall_block->nr_ret, sizeof(in));
			return EXIT_FAILURE;
		}

		if (in != out) {
			fprintf(stderr, "wrote 0x%02x, but read 0x%02x\n",
					in, out);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}




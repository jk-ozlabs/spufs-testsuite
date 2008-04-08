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

#define N_ITERATIONS 102400

/*
 * Create a context, and run it while continuously descheduling it by reading
 * from the regs file.
 */

struct read_thread_info {
	pthread_t	pthread;
	int		fd;
	int		run;
};

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
	stop	0x1337
#endif
static uint32_t spe_program[] = {
	0x00001337, /* stop                           */
};

int main()
{
	int ls_fd, rc, ctx, i;
	struct read_thread_info read_thread;
	char *name = "/spu/02-sched";
	void *ls_map;

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	read_thread.fd = openat(ctx, "regs", O_RDONLY);
	assert(read_thread.fd >= 0);

	ls_fd = openat(ctx, "mem", O_RDWR);
	assert(ls_fd >= 0);

	ls_map = mmap(NULL, LS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			ls_fd, 0);
	assert(ls_map != MAP_FAILED);
	close(ls_fd);

	memcpy(ls_map, spe_program, sizeof(spe_program));

	/* start the reader thread to keep descheduling the context */
	read_thread.run = 1;
	rc = pthread_create(&read_thread.pthread, NULL,
			read_thread_fn, &read_thread);
	assert(!rc);

	for (i = 0; i < N_ITERATIONS; i++) {
		uint32_t entry = 0;

		rc = spu_run(ctx, &entry, NULL);

		if (rc != 0x13370002) {
			fprintf(stderr, "spu_run returned unexepected exit "
					"code 0x%x, npc is 0x%x (%d iter)\n",
					rc, entry, i);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}




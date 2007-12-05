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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>

/* create two processes to run contexts on all SPEs */

static int count_spes(void)
{
	struct stat statbuf;
	int rc;

	rc = stat("/sys/devices/system/spu", &statbuf);
	assert(!rc);

	return statbuf.st_nlink - 2;
}

#if 0
	.long 0x0
	.long 0x0
	.long 0x0
	.long 0x0
start:
	lqa	r0,0x0
	ceqi	r0,r0,0
	brnz	r0,start
	stop	0x1337
#endif
static uint32_t spe_program[] = {
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
/* start: */
	0x30800000, /* lqa     r0,0                   */
	0x7c000000, /* ceqi    r0,r0,0                */
	0x217fff00, /* brnz    r0,10 <start>          */
	0x00001337, /* stop                           */
};

struct spe_thread_info {
	pthread_t	pthread;
	int		ctx;
	int		run_rc;
	char		*ls_map;
};

static void *spe_thread(void *data)
{
	struct spe_thread_info *thread = data;
	unsigned int entry = 0x10;

	thread->run_rc = spu_run(thread->ctx, &entry, NULL);
	if (thread->run_rc < 0)
		perror("spu_run");

	return NULL;
}

int main(void)
{
	int n_spes, i, rc, pid;
	int pipe_fds[2];
	struct spe_thread_info *threads;
	char name[32];


	n_spes = count_spes();
	threads = calloc(n_spes, sizeof(*threads));

	rc = pipe(pipe_fds);
	assert(!rc);

	pid = fork();
	assert(pid >= 0);

	/* child should wait until all parent's SPEs are running */
	if (!pid) {
		char c;
		read(pipe_fds[0], &c, 1);
	}


	for (i = 0; i < n_spes; i++) {
		int ls_fd;

		sprintf(name, "/spu/ctx-%d-%d", pid, i);

		threads[i].ctx = spu_create(name, 0, 0755);
		assert(threads[i].ctx >= 0);

		ls_fd = openat(threads[i].ctx, "mem", O_RDWR);
		assert(ls_fd >= 0);

		threads[i].ls_map = mmap(NULL, LS_SIZE, PROT_READ | PROT_WRITE,
				MAP_SHARED, ls_fd, 0);
		assert(threads[i].ls_map != MAP_FAILED);
		close(ls_fd);

		memcpy(threads[i].ls_map, spe_program, sizeof(spe_program));

		rc = pthread_create(&threads[i].pthread, NULL,
				spe_thread, &threads[i]);
		assert(!rc);
	}

	if (pid) {
		char buf = 'a';
		write(pipe_fds[1], &buf, 1);
		sleep(1);
	}

	for (i = 0; i < n_spes; i++) {
		memset(threads[i].ls_map, 0xff, 0x10);
		rc = pthread_join(threads[i].pthread, NULL);
		assert(!rc);
	}

	return EXIT_SUCCESS;

}

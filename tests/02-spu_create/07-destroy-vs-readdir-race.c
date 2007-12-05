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
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <linux/types.h>
#include <linux/dirent.h>
#include <linux/unistd.h>

#include <test/spu_syscalls.h>

#define ITERATIONS 4096

static void do_creates(const char *name)
{
	int i, rc;

	for (i = 0; i < ITERATIONS; i++) {
		rc = spu_create(name, 0, 0);
		assert(rc >= 0);
		close(rc);
	}
}

static int getdents(unsigned int fd, struct dirent *dirp, unsigned int count)
{
	return syscall(SYS_getdents, fd, dirp, count);
}

static void do_readdirs(void)
{
	int dir, rc;
	struct dirent dirents[10];

	dir = open("/spu", O_RDONLY);
	assert(dir);

	for (;;) {
		rc = getdents(dir, dirents, sizeof(dirents));
		if (rc == 0)
			rc = lseek(dir, 0, SEEK_SET);
		assert(rc >= 0);
	}
}

int main(void)
{
	int i, n_procs;
	const char *fmt = "/spu/07-destroy-vs-readdir-race-%d";
	char **names;
	int *pids;

	n_procs = sysconf(_SC_NPROCESSORS_CONF);

	names = calloc(n_procs, sizeof(*names));
	pids = calloc(n_procs, sizeof(*pids));
	assert(names);
	assert(pids);

	for (i = 0; i < n_procs; i++) {
		names[i] = malloc(30);
		sprintf(names[i], fmt, i);
	}

	for (i = 0; i < n_procs; i++) {
		int pid;

		pid = fork();
		assert(pid >= 0);

		if (pid == 0) {
			if (i % 2)
				do_readdirs();
			else
				do_creates(names[i]);

			return EXIT_SUCCESS;
		} else {
			pids[i] = pid;
		}
	}

	for (i = 0; i < n_procs; i++) {
		/* kill readdir processes */
		if (i % 2)
			kill(pids[i], SIGTERM);

		waitpid(pids[i], NULL, 0);
	}


	return EXIT_SUCCESS;
}

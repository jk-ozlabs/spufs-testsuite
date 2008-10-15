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

/**
 * Test to do a read from the sputrace log, after scheduling a context.
 * We should see a log entry containing (at least) the bind_content message.
 */
#define _ATFILE_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>

#include <test/spu_syscalls.h>

int main(void)
{
	int ctx, trace_fd, rc, len;
	const char *name = "/spu/01-sputrace-read";
	char buf[1024];
	uint32_t entry, status;

	trace_fd = open("/proc/sputrace", O_RDONLY);
	if (trace_fd < 0) {
		perror("open(/proc/sputrace)");
		return EXIT_FAILURE;
	}

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	entry = 0;
	rc = spu_run(ctx, &entry, &status);
	assert(rc >= 0);

	len = read(trace_fd, buf, sizeof(buf));
	if (len <= 0) {
		perror("read");
		return EXIT_FAILURE;
	}

	/* null-terminate and look for an appropriate log entry */
	buf[len-1] = '\0';
	if (!strstr(buf, "spu_bind_context__enter")) {
		fprintf(stderr, "Invalid sputrace log?\n");
		return EXIT_FAILURE;
	}

	close(trace_fd);

	return EXIT_SUCCESS;
}

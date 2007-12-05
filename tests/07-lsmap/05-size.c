/*
 * Testsuite for the Linux SPU filesystem
 *
 *  Copyright (C) 2007 Sony Computer Entertainment Inc.
 *  Copyright 2007 Sony Corporation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* Test to check that file size reported by stat is equal to LS_SIZE */

#define _ATFILE_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <test/hw.h>
#include <test/spu_syscalls.h>

int main(void)
{
	int ctx, lsfd;
	struct stat s;

	ctx = spu_create("/spu/05-size", 0, S_IRUSR | S_IWUSR | S_IXUSR);
	assert(ctx >= 0);

	lsfd = openat(ctx, "mem", O_RDWR);
	assert(lsfd >= 0);

	if (fstat(lsfd, &s)) {
		perror("fstat");
		return EXIT_FAILURE;
	}

	if (s.st_size != LS_SIZE) {
		fprintf(stderr, "filesize of mem = 0x%08lx != 0x%08x\n",
			(long)s.st_size, LS_SIZE);
		return EXIT_FAILURE;
	}

	close(lsfd);

	return EXIT_SUCCESS;
}


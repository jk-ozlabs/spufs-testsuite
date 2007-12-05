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

/* seek to the end of a context's mem file with SEEK_SET, and check that
 * we can't seek past (with SEEK_CUR) that point
 */
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
	off_t off;

	ctx = spu_create("/spu/05-size", 0, S_IRUSR | S_IWUSR | S_IXUSR);
	assert(ctx >= 0);

	lsfd = openat(ctx, "mem", O_RDWR);
	assert(lsfd >= 0);

	off = lseek(lsfd, LS_SIZE, SEEK_SET);
	if (off == (off_t)-1) {
		perror("lseek");
		return EXIT_FAILURE;

	} else if (off != LS_SIZE) {
		fprintf(stderr, "seek to tail by SEEK_END, but return value "
				"0x%08lx != 0x%08x\n", (long)off, LS_SIZE);
		return EXIT_FAILURE;
	}

	off = lseek(lsfd, 1, SEEK_CUR);
	if (off != (off_t)-1) {
		fprintf(stderr, "seek beyond the end of file succeeded, "
				"expecting failure (returned offset=0x%08lx)\n",
				(long)off);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

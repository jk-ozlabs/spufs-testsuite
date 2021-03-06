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

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(void)
{
	struct stat statbuf;
	int rc;

	rc = stat("/proc/sputrace", &statbuf);
	if (rc) {
		perror("stat(/proc/sputrace)");
		return EXIT_FAILURE;
	}

	if (statbuf.st_mode != (S_IFREG | S_IRUSR)) {
		fprintf(stderr, "invalid mode: %04o\n", statbuf.st_mode);
		return EXIT_FAILURE;
	}

	if (statbuf.st_uid != 0) {
		fprintf(stderr, "invalid user: %d\n", statbuf.st_uid);
		return EXIT_FAILURE;
	}

	if (statbuf.st_gid != 0) {
		fprintf(stderr, "invalid group: %d\n", statbuf.st_gid);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

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

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* check that a gang exists before a fd is closed, and that it no longer
 * exists once fd is closed
 */
int check_gang_presence_on_close(const char *gang_name, int fd)
{
	struct stat statbuf;

	if (stat(gang_name, &statbuf)) {
		fprintf(stderr, "gang destroyed too early\n");
		return -1;
	}

	if (close(fd)) {
		perror("close(fd)");
		return -1;
	}

	if (!stat(gang_name, &statbuf)) {
		fprintf(stderr, "gang not destroyed\n");
		return -1;
	}

	if (errno != ENOENT) {
		fprintf(stderr, "Wrong errno (%d) for stat\n", errno);
		return -1;
	}

	return 0;
}

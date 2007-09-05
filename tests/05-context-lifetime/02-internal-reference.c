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
#include <test/utils.h>
#include <test/spu_syscalls.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "gang-presence.h"

/* create a gang, reference it with open(), then destroy the gang. */

int main(void)
{
	int gang, fd;
	char *gang_name;

	gang_name = name_spu_gang();
	gang = spu_create(gang_name, SPU_CREATE_GANG, 0700);
	assert(gang >= 0);

	/* create an extra reference to the gang */
	fd = open(gang_name, O_RDONLY);
	assert(fd >= 0);

	if (close(gang)) {
		perror("close(gang)");
		return EXIT_FAILURE;
	}

	if (check_gang_presence_on_close(gang_name, fd))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

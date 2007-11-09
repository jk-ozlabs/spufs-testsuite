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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/* test for the correct error code (ENODEV) when creating an isolated SPE
 * context on non-isolated hardware
 *
 * At the moment, we assume that we're not running on isolation-enabled
 * hardware, but we may need to remove that assumption in future.
 */

int main(void)
{
	int ctx;

	umask(0);
	ctx = spu_create("/spu/06-isolation-unavailable",
			SPU_CREATE_ISOLATE | SPU_CREATE_NOSCHED,
			0777);

	if (ctx >= 0) {
		fprintf(stderr, "spu_create with SPU_CREATE_ISOLATE flag "
				"succeeded\n");
		close(ctx);
		return EXIT_FAILURE;
	}

	if (errno != ENODEV) {
		fprintf(stderr, "Incorrect errno for SPU_CREATE_ISOLATE: "
				"got %d, expected %d\n", errno, ENODEV);
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}

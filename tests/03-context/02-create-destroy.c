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
#include <libspe2.h>
#include <stdlib.h>
#include <unistd.h>

/* create a context, then destroy it */

int main(void)
{
	spe_context_ptr_t ctx;
	int rc;

	ctx = spe_context_create(0, NULL);

	if (!ctx) {
		perror("spe_context_create");
		return EXIT_FAILURE;
	}

	rc = spe_context_destroy(ctx);

	if (rc) {
		perror("spe_context_destroy");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

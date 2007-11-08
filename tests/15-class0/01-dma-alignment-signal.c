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
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#include <test/spu_syscalls.h>

#include "class0-apps.h"

uint8_t buf[4096] __attribute__((aligned(128)));

siginfo_t siginfo = {
	.si_signo = 0
};

void sighandler(int signal, siginfo_t *s, void *ucontext)
{
	memcpy(&siginfo, s, sizeof(siginfo));
}

int main()
{
	int ctx, rc;
	uint32_t entry;
	char *name = "/spu/01-spe-get";
	struct sigaction action = {
		.sa_sigaction	= sighandler,
		.sa_flags	= SA_SIGINFO
	};

	rc = sigaction(SIGBUS, &action, NULL);
	assert(!rc);

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	entry = class0_load_misaligned_dma_app(ctx, buf);

	rc = spu_run(ctx, &entry, NULL);

	if (rc >= 0) {
		fprintf(stderr, "spu_run succeeded (rc=0x%08x), "
				"expected failure\n", rc);
		return EXIT_FAILURE;
	}

	if (siginfo.si_signo != SIGBUS) {
		fprintf(stderr, "no sigbus received!\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

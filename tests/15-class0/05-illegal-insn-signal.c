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
#include <string.h>
#include <signal.h>

#include <test/spu_syscalls.h>

#include "class0-apps.h"

/* trigger a class 0 interrupt by performing an invalid SPE instruction */
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
	char *name = "/spu/05-illegal-insn-signal";
	int illegal_insn_addr = 0x8;
	struct sigaction action = {
		.sa_sigaction	= sighandler,
		.sa_flags	= SA_SIGINFO
	};

	rc = sigaction(SIGILL, &action, NULL);
	assert(!rc);

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	entry = class0_load_illegal_instruction_app(ctx);
	rc = spu_run(ctx, &entry, NULL);

	if (rc >= 0) {
		fprintf(stderr, "spu_run succeeded (rc=0x%08x), "
				"expected failure\n", rc);
		return EXIT_FAILURE;
	}

	/* npc should be af the illegal instruction */
	if (entry != illegal_insn_addr + 4) {
		fprintf(stderr, "npc at 0x%05x, expected 0x%x\n",
				entry, illegal_insn_addr + 4);
		return EXIT_FAILURE;
	}

	if (siginfo.si_signo != SIGILL) {
		fprintf(stderr, "no sigill received!\n");
		return EXIT_FAILURE;
	}

	if (siginfo.si_addr != (void *)illegal_insn_addr) {
		fprintf(stderr, "siginfo.si_addr = %p, expeted 0x%x",
				siginfo.si_addr, illegal_insn_addr);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

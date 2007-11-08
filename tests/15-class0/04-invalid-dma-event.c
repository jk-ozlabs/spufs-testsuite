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

uint8_t buf[4096] __attribute__((aligned(128)));

int main()
{
	int ctx, rc;
	uint32_t entry, event;
	char *name = "/spu/01-spe-get";

	ctx = spu_create(name, SPU_CREATE_EVENTS_ENABLED, 0755);
	assert(ctx >= 0);

	entry = class0_load_invalid_dma_app(ctx, buf);
	event = 0;

	rc = spu_run(ctx, &entry, &event);

	if (rc != 0) {
		fprintf(stderr, "spu_run returned 0x%08x, expected 0\n", rc);
		return EXIT_FAILURE;
	}

	if (event != SPE_EVENT_INVALID_DMA) {
		fprintf(stderr, "got event 0x%04x, expected 0x%04x\n",
				event, SPE_EVENT_INVALID_DMA);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

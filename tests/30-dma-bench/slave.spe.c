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
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <assert.h>

#include "test.h"
#define ROLE "slave "

volatile int buffer[BUFFER_SIZE] __attribute__ ((aligned (16)));

int main (int spuid, unsigned long long addr, void *envp)
{
	int i;
	DBG("about to report in\n");

	spu_writech(SPU_WrOutMbox, 1);
	DBG("reported in\n");

	DBG("waiting for buffer mark instruction\n");

	/* If we're being read from, fill up the buffer with the sig */
	if (spu_readch(SPU_RdInMbox)) {
		DBG("marking buffer\n");
		for (i = 0; i < BUFFER_SIZE; i++)
			buffer[i] = BUFFER_SIGNATURE;
	} else
		DBG("NOT marking buffer\n");

	/* Write our buffer address to the PPE. */
	spu_writech(SPU_WrOutMbox, (unsigned int)buffer);

	DBG("waiting for termination signal\n");
	/* Wait for a write from the PPE before exiting. */
	spu_readch(SPU_RdInMbox);

	assert(buffer[BUFFER_SIZE - 1] = BUFFER_SIGNATURE);
	
	return 0;
}

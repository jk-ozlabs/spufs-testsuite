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
#define _ATFILE_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>


/**
 * Test both reading and writing of local store while a SPE is running.
 *
 * Initialise the first quadword in local store to zero, and upload SPU
 * code to set it to all ones, and loop while it's not zero.
 *
 * The PPE side checks for the all-ones pattern, then sets the quadword to
 * zero.
 */

void *ppe_thread(void *data)
{
	volatile uint32_t *ls = data;
	int i = 0;

	/* wait for the SPE app to set the first quadword of local store to all
	 * ones */
	while ((ls[0] & ls[1] & ls[2] & ls[3]) != 0xffffffff)
		i++;

	/* set the first quadword to all zeros */
	ls[0] = ls[1] = ls[2] = ls[3] = 0x00000000;

	printf("%s: %d loops\n", __func__, i);

	return NULL;
}

static uint32_t spe_app[] = {
	0x40ffff83, /* il      $3,-1		*/
	0x20800003, /* stqa    $3,0		*/
	0x30800003, /* lqa     $3,0		*/
	0x217fff83, /* brnz    $3,(. - 4)	*/
	0x00001337, /* stop    0x1337		*/
};

int main(void)
{
	int ctx, ls_fd, rc, ls_buf_size;
	const char *name = "/spu/ppe-running";
	uint32_t *ls_map, entry, status;
	pthread_t thread;

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	ls_fd = openat(ctx, "mem", O_RDWR);
	if (ls_fd < 0) {
		perror("openat: mem");
		return EXIT_FAILURE;
	}

	ls_map = mmap(NULL, LS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			ls_fd, 0);
	if (ls_map == MAP_FAILED) {
		perror("mmap");
		return EXIT_FAILURE;
	}

	/* we need a gap for a quadword */
	ls_buf_size = 4;

	memset(ls_map, 0, ls_buf_size * sizeof(uint32_t));
	memcpy(ls_map + ls_buf_size, spe_app, sizeof(spe_app));

	rc = pthread_create(&thread, NULL, ppe_thread, ls_map);
	if (rc) {
		perror("pthread_create");
		return EXIT_FAILURE;
	}

	/* give the ppe thread time to schedule */
	usleep(1);

	entry = ls_buf_size * sizeof(uint32_t);
	rc = spu_run(ctx, &entry, &status);

	if (rc < 0) {
		perror("spu_run");
		return EXIT_FAILURE;
	}

	if (!(rc & 0x2)) {
		fprintf(stderr, "spu didn't stop-and-signal? rc = 0x%x\n", rc);
		return EXIT_FAILURE;
	}

	if (rc >> 16 != 0x1337) {
		fprintf(stderr, "spu executed different stop instruction? "
				"rc = 0x%x\n", rc);
		return EXIT_FAILURE;
	}

	printf("npc: 0x%x, status: 0x%x\n", entry, rc);

	if (pthread_join(thread, NULL)) {
		perror("pthread_join");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

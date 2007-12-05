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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>

#define nr_msgs	(102400)
#define sched_interval (16)

struct spe_thread_info {
	pthread_t	pthread;
	int		ctx;
	int		run_rc;
	uint32_t	entry;
};
static int thread_abort;

static void *spe_thread(void *data)
{
	struct spe_thread_info *thread = data;

	thread->run_rc = spu_run(thread->ctx, &thread->entry, NULL);
	if (thread->run_rc < 0)
		perror("spu_run");

	thread_abort = 1;

	return NULL;
}

static uint32_t spe_program[] = {
	/* set r2 to the stop message (0xffffffff 00000000 00000000 00000000) */
	0x32f80002, /* fsmbi   r2,0xf000    */

	/* set r0 to 0 */
	0x40800000, /* il      r0,0         */

	/* read a message into r1 */
	0x01a00e81, /* rdch    r1,rch29     */

	/* compare the message to the stop message. If they're equal, stop */
	0x78008083, /* ceq     r3,r1,r2     */
	0x20000103, /* brz     r3,0x18 # 18 */
	0x00001337, /* stop    0x1337       */

	/* compare to the message count (and increment). If they're equal,
	 * branch back up */
	0x78000083, /* ceq     r3,r1,r0     */
	0x1c004000, /* ai      r0,r0,1      */
	0x217ffd03, /* brnz    r3,0xc       */
	0x00000000, /* stop                 */
};

int main()
{
	int ls_fd, mbox_fd, reg_fd, rc, i;
	uint32_t msg, reg;
	struct spe_thread_info thread;
	char *name = "/spu/03-mbox-multiple+sched";

	thread.ctx = spu_create(name, 0, 0755);
	assert(thread.ctx >= 0);

	ls_fd = openat(thread.ctx, "mem", O_RDWR);
	assert(ls_fd >= 0);

	mbox_fd = openat(thread.ctx, "wbox", O_WRONLY);
	if (mbox_fd <= 0) {
		perror("open:wbox");
		return EXIT_FAILURE;
	}

	reg_fd = openat(thread.ctx, "regs", O_RDWR);
	assert(reg_fd >= 0);

	rc = write(ls_fd, spe_program, sizeof(spe_program));
	assert(rc = sizeof(spe_program));

	thread.entry = 0;

	rc = pthread_create(&thread.pthread, NULL, spe_thread, &thread);
	assert(rc == 0);

	for (i = 0; i < nr_msgs; i++) {
		msg = i;

		rc = write(mbox_fd, &msg, sizeof(msg));
		if (rc != sizeof(msg)) {
			perror("write");
			return EXIT_FAILURE;
		}

		if (thread_abort)
			break;

		/* reading the "regs" file requires that the context be
		 * scheduled out */
		if (i % sched_interval == (sched_interval - 1)) {
			rc = read(reg_fd, &reg, sizeof(reg));
			assert(rc == sizeof(reg));
			rc = lseek(reg_fd, 0, SEEK_SET);
			assert(rc == 0);
		}
	}

	if (i < nr_msgs) {
		fprintf(stderr, "SPU program exited early\n");
		return EXIT_FAILURE;
	}

	/* send the stop message */
	msg = 0xffffffff;
	rc = write(mbox_fd, &msg, sizeof(msg));
	if (rc != sizeof(msg)) {
		perror("write");
		return EXIT_FAILURE;
	}

	rc = pthread_join(thread.pthread, NULL);


	if (thread.run_rc != 0x13370002) {
		fprintf(stderr, "Incorrect stop code: 0x%x\n", thread.run_rc);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

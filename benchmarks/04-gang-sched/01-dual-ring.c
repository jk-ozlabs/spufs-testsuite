/*
 * Testsuite for the Linux SPU filesystem
 *
 * Copyright (C) IBM Corporation, 2008
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

/**
 * Gang scheduling benchmark: create two gangs with a high dependency
 * between members. Contexts within a gang pass a message in a ring formation.
 *
 * Benchmark the time required to perform ITERATIONS cycles of message-passing
 */

#define _ATFILE_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>

#include <test/spu_syscalls.h>
#include <test/timing.h>
#include <test/hw.h>

#define ITERATIONS 0x1000000
#define NR_GANGS    2

#if 0
sig_addr:
	/* set to the address of the next SPE's signotify register */
	.space 16
iters:
	/* set to ITERATIONS */
	.space 16
buf:
	/* our local buffer for the DMA sndsig command. The actual
	 * signal word lies at buf + 0xc, as the remote signotify register
	 * is at an 0xc offset. */
	.space 0xc
	.long 0x0

.globl _start
_start:
	lqa	r1,sig_addr	/* r1 = signotify addr, low 32 bits */
	shlqbyi	r2,r1,4		/* r1 = signotify addr, high 32 bits */
	lqa	r3,iters	/* r3 = counter */
	il	r4,buf+0xc	/* local DMA buffer */
	il	r5,0		/* DMA tag */
	il	r6,4		/* DMA size */
	il	r7,0xa0		/* DMA command ('sndsig') */

	il	r8,1		/* mask for DMA tag 0 */
	il	r9,1		/* tag status: 1: any completion */
	ila	r11,0x00010203	/* pattern for shufb */

loop:
	/* wait for signal from signotify 1 */
	rdch	r10,ch3

	/* increment incoming message and set local buffer */
	ai	r10,r10,1
	shufb	r10,r10,r10,r11
	stqa	r10,buf

	/* write to next SPE's signotify register */
	wrch	ch16,r4
	wrch	ch17,r1
	wrch	ch18,r2
	wrch	ch19,r6
	wrch	ch20,r5
	wrch	ch21,r7

	/* wait for DMA completion */
	wrch	ch22,r8
	wrch	ch23,r9
	rdch	r12,ch24

	/* decrement counter, loop if non-zero */
	ai	r3,r3,-1
	brnz	r3,loop

	stop	0x1337
#endif
static uint32_t spe_program[] = {
/* sig_addr: */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
/* iters: */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
/* buf: */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
/* _start: */
	0x30800001, /* lqa     r1,0                   */
	0x3fe10082, /* shlqbyi r2,r1,4                */
	0x30800203, /* lqa     r3,10 <iters>          */
	0x40801604, /* il      r4,44   # 2c           */
	0x40800005, /* il      r5,0                   */
	0x40800206, /* il      r6,4                   */
	0x40805007, /* il      r7,160  # a0           */
	0x40800088, /* il      r8,1                   */
	0x40800089, /* il      r9,1                   */
	0x4281018b, /* ila     r11,10203 <__bss_start */
/* loop: */
	0x01a0018a, /* rdch    r10,$ch3               */
	0x1c00450a, /* ai      r10,r10,1              */
	0xb142850b, /* shufb   r10,r10,r10,r11        */
	0x2080040a, /* stqa    r10,20 <buf>           */
	0x21a00804, /* wrch    $ch16,r4               */
	0x21a00881, /* wrch    $ch17,r1               */
	0x21a00902, /* wrch    $ch18,r2               */
	0x21a00986, /* wrch    $ch19,r6               */
	0x21a00a05, /* wrch    $ch20,r5               */
	0x21a00a87, /* wrch    $ch21,r7               */
	0x21a00b08, /* wrch    $ch22,r8               */
	0x21a00b89, /* wrch    $ch23,r9               */
	0x01a00c0c, /* rdch    r12,$ch24              */
	0x1cffc183, /* ai      r3,r3,-1               */
	0x217ff903, /* brnz    r3,58 <loop>    # 58   */
	0x00001337, /* stop                           */
};
static uint32_t spe_program_entry = 0x00000030;

struct spe_thread {
	int ctx;
	pthread_t pthread;
	volatile void *psmap;
	void *lsmap;
};

struct spe_gang {
	int fd, nr_spes;
	struct spe_thread *threads;
};

void *spe_thread_fn(void *arg)
{
	struct spe_thread *spe_thread = arg;
	uint32_t entry = spe_program_entry;
	int rc;

	rc = spu_run(spe_thread->ctx, &entry, NULL);
	if (rc != 0x13370002)
		perror("spu_run");

	return NULL;
}

static int create_gang(const char *gang_name, int nr_spes,
		struct spe_gang *gang)
{
	int rc, i;
	const char *name_fmt = "%s/01-dual-ring.%d";
	struct spe_thread *threads;

	threads = calloc(nr_spes, sizeof(*threads));

	gang->nr_spes = nr_spes;
	gang->fd = spu_create(gang_name, SPU_CREATE_GANG, 0755);
	if (gang->fd < 0) {
		perror("spu_create(SPU_CREATE_GANG)");
		return EXIT_FAILURE;
	}

	/* initial gang setup: create contexts, map localstore and
	 * problem-state areas */
	for (i = 0; i < nr_spes; i++) {
		char name[40];
		int psmap_fd, ls_fd;

		sprintf(name, name_fmt, gang_name, i);
		threads[i].ctx = spu_create(name, 0, 0755);

		if (threads[i].ctx < 0) {
			perror("spu_create");
			return EXIT_FAILURE;
		}

		/* map problem-state area */
		psmap_fd = openat(threads[i].ctx, "psmap", O_RDWR);
		assert(psmap_fd >= 0);

		threads[i].psmap = mmap(NULL, PSMAP_SIZE,
				PROT_READ | PROT_WRITE, MAP_SHARED,
				psmap_fd, 0);
		if (threads[i].psmap == MAP_FAILED) {
			perror("mmap(psmap)");
			return EXIT_FAILURE;
		}

		close(psmap_fd);

		/* map local store */
		ls_fd = openat(threads[i].ctx, "mem", O_RDWR);
		assert(ls_fd >= 0);

		threads[i].lsmap = mmap(NULL, LS_SIZE, PROT_READ | PROT_WRITE,
				MAP_SHARED, ls_fd, 0);
		if (threads[1].lsmap == MAP_FAILED) {
			perror("mmap(mem)");
			return EXIT_FAILURE;
		}
		close(ls_fd);
	}

	/* now that we have all the psmaps set up, set the SPE-specific
	 * parameters in local store */
	for (i = 0; i < nr_spes; i++) {
		uint32_t *ls = threads[i].lsmap;
		uint64_t next_psmap =
			(unsigned long)threads[(i + 1) % nr_spes].psmap +
			PSMAP_SIGNAL1_OFFSET;


		memcpy(ls, spe_program, sizeof(spe_program));

		ls[0] = next_psmap >> 32;
		ls[1] = next_psmap & 0xffffffff;

		ls[4] = ITERATIONS;

	}

	/* start threads... */
	for (i = 0; i < nr_spes; i++) {
		rc = pthread_create(&threads[i].pthread, NULL,
				spe_thread_fn, &threads[i]);

		if (rc) {
			perror("pthread_create");
			return EXIT_FAILURE;
		}
	}

	gang->threads = threads;
	return EXIT_SUCCESS;
}

static void run_gang(struct spe_gang *gang)
{
	*(uint32_t *)(gang->threads[0].psmap + PSMAP_SIGNAL1_OFFSET) = 0;
}

static int wait_for_gang_completion(struct spe_gang *gang)
{
	int i, rc;

	for (i = 0; i < gang->nr_spes; i++) {
		rc = pthread_join(gang->threads[i].pthread, NULL);
		if (rc)
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main(void)
{
	const char *gang_fmt = "/spu/01-dual-ring.%d";
	struct spe_gang gangs[NR_GANGS];
	int i, rc, nr_spes;
	struct timeval tv;

	nr_spes = count_spes();

	for (i = 0; i < NR_GANGS; i++) {
		char name[40];
		sprintf(name, gang_fmt, i);

		rc = create_gang(name, nr_spes, &gangs[i]);
		if (rc)
			return EXIT_FAILURE;
	}

	timer_start(&tv);

	for (i = 0; i < NR_GANGS; i++)
		run_gang(&gangs[i]);

	for (i = 0; i < NR_GANGS; i++)
		wait_for_gang_completion(&gangs[i]);

	timer_stop_and_print(&tv);

	return EXIT_SUCCESS;
}

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

#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>


#include <test/hw.h>


#if 0
	nop
	nop
	.long 0xa0000000
#endif
static uint32_t illegal_instruction_program[] = {
	0x40200000, /* nop     r0                     */
	0x40200000, /* nop     r0                     */
	0xa0000000, /* .long 0xa0000000               */
};


#if 0
	/* area for PPE buffer address, stored in the first two words */
	.long 0x0
	.long 0x0
	.long 0x0
	.long 0x0

start:
	il	r0,0x1000	/* LS address		*/
	lqa	r1,0		/* PPE address		*/
	il	r2,0x1000	/* transfer size	*/
	il	r3,0		/* tag group ID		*/
	il	r4,0x40		/* command ('get')	*/

	/* do dma */
	wrch	ch16,r0
	wrch	ch17,r1
	shlqbyi	r1,r1,4
	wrch	ch18,r1
	wrch	ch19,r2
	wrch	ch20,r3
	wrch	ch21,r4

	/* wait for completion */
	il	r5,1	/* mask for tag 0 */
	il	r6,1	/* 0x1: any completion */

	wrch	ch22,r5
	wrch	ch23,r6

	rdch	r7,ch24
	ceqi	r8,r5,1
	brnz	r8,exit		/* error if our tag (1) isn't complete */
	stop	0x0
exit:
	stop	0x1337

#endif
uint32_t misaligned_dma_program[] = {
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
/* start: */
	0x40880000, /* il      r0,4096 # 1000         */
	0x30800001, /* lqa     r1,0                   */
	0x40880002, /* il      r2,4096 # 1000         */
	0x40800003, /* il      r3,0                   */
	0x40802004, /* il      r4,64   # 40           */
	0x21a00800, /* wrch    $ch16,r0               */
	0x21a00881, /* wrch    $ch17,r1               */
	0x3fe10081, /* shlqbyi r1,r1,4                */
	0x21a00901, /* wrch    $ch18,r1               */
	0x21a00982, /* wrch    $ch19,r2               */
	0x21a00a03, /* wrch    $ch20,r3               */
	0x21a00a84, /* wrch    $ch21,r4               */
	0x40800085, /* il      r5,1                   */
	0x40800086, /* il      r6,1                   */
	0x21a00b05, /* wrch    $ch22,r5               */
	0x21a00b86, /* wrch    $ch23,r6               */
	0x01a00c07, /* rdch    r7,$ch24               */
	0x7c004288, /* ceqi    r8,r5,1                */
	0x21000108, /* brnz    r8,60 <exit>    # 60   */
	0x00000000, /* stop                           */
/* exit: */
	0x00001337, /* stop                           */
};

#if 0
/* area for PPE buffer address, stored in the first two words */
.long 0x0
.long 0x0
.long 0x0
.long 0x0

start:
	il	r0,0x1000	/* LS address		*/
	lqa	r1,0		/* PPE address		*/
	il	r2,0x1000	/* transfer size	*/
	il	r3,0		/* tag group ID		*/
	il	r4,0x100	/* invalid command	*/

	/* do dma */
	wrch	ch16,r0
	wrch	ch17,r1
	shlqbyi	r1,r1,4
	wrch	ch18,r1
	wrch	ch19,r2
	wrch	ch20,r3
	wrch	ch21,r4

	/* wait for completion */
	il	r5,1	/* mask for tag 0 */
	il	r6,1	/* 0x1: any completion */

	wrch	ch22,r5
	wrch	ch23,r6

	rdch	r7,ch24
	ceqi	r8,r5,1
	brnz	r8,exit		/* error if our tag (1) isn't complete */
	stop	0x0
exit:
	stop	0x1337
#endif
static uint32_t invalid_dma_program[] = {
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
	0x00000000, /* stop                           */
/* start: */
	0x40880000, /* il      r0,4096 # 1000         */
	0x30800001, /* lqa     r1,0                   */
	0x40880002, /* il      r2,4096 # 1000         */
	0x40800003, /* il      r3,0                   */
	0x40808004, /* il      r4,256  # 100          */
	0x21a00800, /* wrch    $ch16,r0               */
	0x21a00881, /* wrch    $ch17,r1               */
	0x3fe10081, /* shlqbyi r1,r1,4                */
	0x21a00901, /* wrch    $ch18,r1               */
	0x21a00982, /* wrch    $ch19,r2               */
	0x21a00a03, /* wrch    $ch20,r3               */
	0x21a00a84, /* wrch    $ch21,r4               */
	0x40800085, /* il      r5,1                   */
	0x40800086, /* il      r6,1                   */
	0x21a00b05, /* wrch    $ch22,r5               */
	0x21a00b86, /* wrch    $ch23,r6               */
	0x01a00c07, /* rdch    r7,$ch24               */
	0x7c004288, /* ceqi    r8,r5,1                */
	0x21000108, /* brnz    r8,60 <exit>    # 60   */
	0x00000000, /* stop                           */
/* exit: */
	0x00001337, /* stop                           */
};


static char *load_program(int ctx, uint32_t *program, int size)
{
	int ls_fd;
	char *ls_map;

	ls_fd = openat(ctx, "mem", O_RDWR);
	assert(ls_fd);

	ls_map = mmap(NULL, LS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			ls_fd, 0);
	assert(ls_map != MAP_FAILED);

	memcpy(ls_map, program, size);

	return ls_map;
}

int class0_load_illegal_instruction_app(int ctx)
{
	load_program(ctx, illegal_instruction_program,
			sizeof(illegal_instruction_program));
	return 0;
}

int class0_load_misaligned_dma_app(int ctx, void *buf)
{
	char *ls_map;

	ls_map = load_program(ctx, misaligned_dma_program,
			sizeof(misaligned_dma_program));

	*((uint64_t *)(ls_map)) = (uint64_t)((unsigned long)buf) + 1;

	return 0x10;
}

int class0_load_invalid_dma_app(int ctx, void *buf)
{
	char *ls_map;

	ls_map = load_program(ctx, invalid_dma_program,
			sizeof(invalid_dma_program));

	*((uint64_t *)(ls_map)) = (uint64_t)(unsigned long)(buf);

	return 0x10;
}


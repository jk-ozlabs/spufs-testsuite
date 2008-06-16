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
#define _ATFILE_SOURCE

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include <pthread.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>

#include <slowfs/slowfs.h>

#define SLOWFS_BINARY "lib/slowfs/slowfs"
#define SLOWFS_DIR "mnt"
#define SLOWFS_FILE "mnt/slowfs"

/*
 * Test to context switch in the middle of a page-fault caused by a SPE DMA
 *
 * Using the slowfs filesystem, we mmap a file that takes one second to
 * service the mmap fault. During this second, we schedule the context
 * out then back in.
 */


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
static uint32_t spe_program[] = {
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

static struct slowfs *slowfs;

static int map_slowfs_file(void **map, int *len)
{
	int rc, fd;
	struct stat statbuf;

	rc = mkdir(SLOWFS_DIR, 0755);
	if (rc) {
		perror("mkdir");
		return rc;
	}

	slowfs = slowfs_mount(SLOWFS_DIR);
	if (!slowfs) {
		fprintf(stderr, "failed to mount slowfs on %s\n", SLOWFS_DIR);
		return -1;
	}

	fd = open(SLOWFS_FILE, O_RDONLY);
	if (fd < 0) {
		perror("open");
		rc = fd;
		goto err_umount;
	}

	rc = fstat(fd, &statbuf);
	if (rc) {
		perror("fstat");
		goto err_close;
	}

	*map = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (*map == MAP_FAILED) {
		perror("mmap");
		rc = errno;
		goto err_close;
	}

	close(fd);

	*len = statbuf.st_size;

	return 0;

err_close:
	close(fd);
err_umount:
	slowfs_unmount(slowfs);
	return rc;

}

static void unmap_slow_file(void *map, int len)
{
	if (map != MAP_FAILED)
		munmap(map, len);

	slowfs_unmount(slowfs);
	rmdir(SLOWFS_DIR);
}

struct spe_thread_info {
	pthread_t	pthread;
	int		ctx;
	int		run_rc;
	uint32_t	entry;
};

static void *spe_thread(void *data)
{
	struct spe_thread_info *thread = data;

	thread->run_rc = spu_run(thread->ctx, &thread->entry, NULL);
	if (thread->run_rc < 0)
		perror("spu_run");

	return NULL;
}

int main(void)
{
	int rc;
	int ls_fd, regs_fd, slow_map_len = 0;
	uint8_t *ls_map;
	void *slow_map = NULL;
	struct spe_thread_info thread;
	char buf, *name = "/spu/15-schedule-during-fault";

	thread.ctx = spu_create(name, 0, 0755);
	assert(thread.ctx >= 0);

	ls_fd = openat(thread.ctx, "mem", O_RDWR);
	assert(ls_fd >= 0);

	regs_fd = openat(thread.ctx, "regs", O_RDWR);
	assert(regs_fd >= 0);

	ls_map = mmap(NULL, LS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			ls_fd, 0);
	assert(ls_map != MAP_FAILED);

	memcpy(ls_map, spe_program, sizeof(spe_program));

	if (map_slowfs_file(&slow_map, &slow_map_len))
		return EXIT_FAILURE;

	/* set the PPE address at the first 2 words of LS memory */
	*((uint64_t *)(ls_map)) = (unsigned long)slow_map;
	thread.entry = 0x10;

	rc = pthread_create(&thread.pthread, NULL, spe_thread, &thread);
	if (rc) {
		perror("pthread_create");
		goto out_unmap;
	}

	sleep(1);

	/* trigger a reschedule by reading from the regs file */
	rc = read(regs_fd, &buf, sizeof(buf));
	assert(rc == sizeof(buf));

	rc = pthread_join(thread.pthread, NULL);
	if (rc) {
		perror("pthread_join");
		goto out_unmap;
	}

	if (thread.run_rc != 0x13370002) {
		fprintf(stderr, "spu_run return code 0x%08x, expectng 0x%08x\n",
				thread.run_rc, 0x1337002);
	} else {
		rc = 0;
	}

out_unmap:
	unmap_slow_file(slow_map, slow_map_len);
	return rc;
}



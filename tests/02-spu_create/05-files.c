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
#include <test/utils.h>
#include <test/spu_syscalls.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <test/hw.h>

#define SPU_DMA_INFO_SIZE (5 * sizeof(uint64_t) + 16 * 4 * sizeof(uint64_t))
#define SPU_PROXYDMA_INFO_SIZE (3 * sizeof(uint64_t) + 8 * 4 * sizeof(uint64_t))

struct spufs_file {
	const char	*name;
	mode_t		mode;
	size_t		size;
} spufs_files[] = {
	{ "capabilities",	0100444, },
	{ "mem",		0100666, LS_SIZE },
	{ "regs",		0100666, 16 * 128 },
	{ "mbox",		0100444, },
	{ "ibox",		0100444, },
	{ "wbox",		0100222, },
	{ "mbox_stat",		0100444, sizeof(uint32_t) },
	{ "ibox_stat",		0100444, sizeof(uint32_t) },
	{ "wbox_stat",		0100444, sizeof(uint32_t) },
	{ "signal1",		0100666, },
	{ "signal2",		0100666, },
	{ "signal1_type",	0100666, },
	{ "signal2_type",	0100666, },
	{ "cntl",		0100666, },
	{ "fpcr",		0100666, 16 },
	{ "lslr",		0100444, },
	{ "mfc",		0100666, },
	{ "mss",		0100666, },
	{ "npc",		0100666, },
	{ "srr0",		0100666, },
	{ "decr",		0100666, },
	{ "decr_status",	0100666, },
	{ "event_mask",		0100666, },
	{ "event_status",	0100444, },
	{ "psmap",		0100666, 0x20000 },
	{ "phys-id",		0100666, },
	{ "object-id",		0100666, },
	{ "mbox_info",		0100444, sizeof(uint32_t) },
	{ "ibox_info",		0100444, sizeof(uint32_t) },
	{ "wbox_info",		0100444, sizeof(uint32_t) },
	{ "dma_info",		0100444, SPU_DMA_INFO_SIZE },
	{ "proxydma_info",	0100444, SPU_PROXYDMA_INFO_SIZE },
	{ "tid",		0100444, },
	{ "stat",		0100444, },
	{ NULL,			0    }
};

int main(void)
{
	int ctx;
	struct stat stat;
	struct spufs_file *file;

	umask(0);
	ctx = spu_create("/spu/05-files", 0, 0777);
	assert(ctx >= 0);

	for (file = spufs_files; file->name; file++) {

		if (fstatat(ctx, file->name, &stat, 0)) {
			fprintf(stderr, "can't stat context file %s: %s\n",
					file->name, strerror(errno));
			return EXIT_FAILURE;
		}

		if (stat.st_mode != file->mode) {
			fprintf(stderr, "file %s: mode %o doesn't match %o\n",
					file->name, stat.st_mode, file->mode);
			return EXIT_FAILURE;
		}

		if (file->size != stat.st_size) {
			fprintf(stderr, "file %s: size %ld doesn't match %zu\n",
					file->name, stat.st_size, file->size);
			return EXIT_FAILURE;
		}
	}

	close(ctx);

	return EXIT_SUCCESS;
}

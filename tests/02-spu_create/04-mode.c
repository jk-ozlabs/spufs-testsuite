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

#define TEST_MODES (	S_IRUSR | S_IWUSR | S_IXUSR | \
			S_IRGRP | S_IWGRP | S_IXGRP | \
			S_IROTH | S_IWOTH | S_IXOTH )

int main(void)
{
	int ctx, mode;
	char *name;
	struct stat stat;

	printf("%s: checking modes: 0%04o\n", __FILE__, TEST_MODES);
	fflush(stdout);

	umask(0);

	for (mode = 0; mode <= TEST_MODES; mode++) {
		if ((mode & TEST_MODES) != mode)
			continue;

		name = name_spu_context(NULL);

		ctx = spu_create(name, 0, mode);
		assert(ctx >= 0);

		if (fstat(ctx, &stat)) {
			perror("fstat");
			return EXIT_FAILURE;
		}

		stat.st_mode &= TEST_MODES;

		if (stat.st_mode != mode) {
			fprintf(stderr, "incorrect mode 0%04o, "
					"expecting 0%04o\n",
					stat.st_mode, mode);
			return EXIT_FAILURE;
		}

		close(ctx);
		free(name);
	}


	return EXIT_SUCCESS;
}

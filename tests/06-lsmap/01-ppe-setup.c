#define _ATFILE_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>

#include <test/spu_syscalls.h>

int main(void)
{
	int ctx, ls_fd, rc;
	const char *name = "/spu/ppe-access";
	uint32_t buf, entry, status;

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	ls_fd = openat(ctx, "mem", O_RDWR);
	if (ls_fd < 0) {
		perror("openat");
		return EXIT_FAILURE;
	}

	buf = 0x00001337; /* stop 0x1337 */
	rc = write(ls_fd, &buf, sizeof(buf));
	if (rc != sizeof(buf)) {
		perror("write");
		return EXIT_FAILURE;
	}

	entry = 0;
	rc = spu_run(ctx, &entry, &status);

	if (rc < 0) {
		perror("spu_run");
		return EXIT_FAILURE;
	}

	if (entry != 0x4) {
		fprintf(stderr, "spu finished exection at 0x%x, "
				"expected 0x4\n", entry);
		return EXIT_FAILURE;
	}

	if (!(rc & 0x2)) {
		fprintf(stderr, "spu didn't stop-and-signal?\n");
		return EXIT_FAILURE;
	}

	if (rc >> 16 != 0x1337) {
		fprintf(stderr, "spu executed different stop instruction?\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

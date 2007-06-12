#define _ATFILE_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>

#include <test/spu_syscalls.h>

int main(void)
{
	int rc, fd;
	uint32_t entry, status;

	fd = open("/dev/null", O_RDWR);
	assert(fd >= 0);

	rc = spu_run(fd, &entry, &status);

	if (rc != -1) {
		fprintf(stderr, "spu_run didn't fail\n");
		return EXIT_FAILURE;
	}

	if (errno != EINVAL) {
		fprintf(stderr, "spu_run failed with wrong errno (%d)\n",
				errno);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

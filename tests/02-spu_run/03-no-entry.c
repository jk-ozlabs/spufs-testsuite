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
	int rc, ctx;
	const char *name = "/spu/03-no-entry";
	uint32_t status;

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	rc = spu_run(ctx, NULL, &status);

	if (rc != -1) {
		fprintf(stderr, "spu_run didn't fail\n");
		return EXIT_FAILURE;
	}

	if (errno != EFAULT) {
		fprintf(stderr, "spu_run failed with wrong errno (%d)\n",
				errno);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

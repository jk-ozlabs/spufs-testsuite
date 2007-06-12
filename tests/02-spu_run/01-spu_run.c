#define _ATFILE_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>

#include <test/spu_syscalls.h>

int main(void)
{
	int ctx, rc;
	const char *name = "/spu/" __FILE__;
	uint32_t entry, status;

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	entry = 0;
	rc = spu_run(ctx, &entry, &status);

	if (rc < 0) {
		perror("spu_run");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

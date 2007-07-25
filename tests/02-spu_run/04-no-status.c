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
	const char *name = "/spu/04-no-status";
	uint32_t entry;

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	entry = 0;
	rc = spu_run(ctx, &entry, NULL);

	if (rc < 0) {
		perror("spu_run(no status)");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

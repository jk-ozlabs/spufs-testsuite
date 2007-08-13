
#include <test/spu_syscalls.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>

int main(void)
{
	int gang, ctx, bad_fd;
	const char *names[] = {"/spu/gang-01/",
			       "/spu/gang-01/02-affinity-create-spu.1"};

	gang = spu_create(names[0], SPU_CREATE_GANG, 0755);
	assert(gang >= 0);

	bad_fd = open("/dev/null", O_RDONLY);
	assert(bad_fd >= 0);
	close(bad_fd);

	ctx = spu_create(names[1], SPU_CREATE_AFFINITY_SPU, 0, bad_fd);

	if (ctx > 0) {
		fprintf(stderr, "expected spu_create to fail!\n");
		return EXIT_FAILURE;
	}

	if (errno != EBADF) {
		fprintf(stderr, "Bad errno: got %d, expected EBADF\n", errno);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

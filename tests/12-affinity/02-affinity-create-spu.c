#include <test/spu_syscalls.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int main(void)
{
	int ctx[2];
	const char *names[] = {"/spu/01-affinity-create-spu.1",
			       "/spu/01-affinity-create-spu.2"};

	ctx[0] = spu_create(names[0], 0, 0);
	assert(ctx[0]);

	ctx[1] = spu_create(names[1], SPU_CREATE_AFFINITY_SPU, 0, ctx[0]);

	if (ctx[1] < 0) {
		perror("spu_create");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

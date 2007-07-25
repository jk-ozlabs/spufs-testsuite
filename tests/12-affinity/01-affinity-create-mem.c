#include <test/spu_syscalls.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int main(void)
{
	const char *gang_name = "/spu/01-gang";
	const char *ctx_name = "/spu/01-gang/01-affinity-create-mem";
	int ctx, gang;

	gang = spu_create(gang_name, SPU_CREATE_GANG, 0755);
	assert(gang >= 0);

	ctx = spu_create(ctx_name, SPU_CREATE_AFFINITY_MEM, 0);

	if (ctx < 0) {
		perror("spu_create");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

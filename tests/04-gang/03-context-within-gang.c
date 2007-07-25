#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <test/spu_syscalls.h>

/* create a gang */

int main(void)
{
	const char *gang_name = "/spu/01-gang";
	const char *ctx_name = "/spu/01-gang/01-ctx";
	int gang, ctx;

	gang = spu_create(gang_name, SPU_CREATE_GANG, 0755);
	assert(gang >= 0);

	ctx = spu_create(ctx_name, 0, 0755);

	if (ctx < 0) {
		perror("spu_create");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

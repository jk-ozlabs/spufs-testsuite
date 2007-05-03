#include <test/utils.h>
#include <test/spu_syscalls.h>
#include <stdlib.h>
#include <unistd.h>

/* create a gang with a context, then destroy the gang */

int main(void)
{
	int gang, ctx;
	char *ctx_name, *gang_name;

	gang_name = name_spu_gang();
	gang = spu_create(gang_name, SPU_CREATE_GANG, 0700);
	assert(gang != -1);

	ctx_name = name_spu_context(gang_name);
	printf("creating context %s\n", ctx_name);
	ctx = spu_create(ctx_name, 0, 0700);
	if (ctx == -1) {
		perror("spu_create");
		return EXIT_FAILURE;
	}
	assert(ctx != -1);

	close(gang);

	return EXIT_SUCCESS;
}

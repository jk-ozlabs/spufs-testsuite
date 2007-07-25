#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <test/spu_syscalls.h>

/* create a gang, then destroy it */

int main(void)
{
	const char *name = "/spu/02-gang";
	int gang;

	gang = spu_create(name, SPU_CREATE_GANG, 0755);
	assert(gang >= 0);

	if (close(gang)) {
		perror("close");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

#include <stdlib.h>
#include <unistd.h>

#include <test/spu_syscalls.h>

/* create a gang */

int main(void)
{
	const char *name = "/spu/01-gang";
	int gang;

	gang = spu_create(name, SPU_CREATE_GANG, 0755);

	if (gang < 0) {
		perror("spu_create");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

#include <test/spu_syscalls.h>
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
	int rc;
	const char *name = "/spu/01-create";

	rc = spu_create(name, 0, 0);

	if (rc < 0) {
		perror("spu_create");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

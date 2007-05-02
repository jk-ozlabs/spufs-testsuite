#include <test/spu_syscalls.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

int main(void)
{
	int rc;

	rc = spu_create(NULL, 0, 0);

	assert(rc == -1);
	if (errno != EFAULT) {
		fprintf(stderr, "expected ENOENT(%d), got %d\n", ENOENT, errno);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

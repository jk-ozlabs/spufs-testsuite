#include <test/spu_syscalls.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

int main(void)
{
	int rc;

	rc = spu_create(NULL, 0, 0);

	assert(rc == -1);
	assert(errno == ENOENT);

	return EXIT_SUCCESS;
}

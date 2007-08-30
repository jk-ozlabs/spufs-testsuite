#define _ATFILE_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include <test/spu_syscalls.h>

int main(void)
{
	int i, ctx[16];
	char name[256];

	for (i = 0; i < 16; i++) {
		snprintf(name, 256, "/spu/10-coredump-%d-%d", getpid(), i);
		ctx[i] = spu_create(name, 0, 0755);
		assert(ctx[i] >= 0);
	}

	printf("%d\n", getpid());
	fflush(stdout);

	*(int *)NULL = 1;

	return EXIT_SUCCESS;
}

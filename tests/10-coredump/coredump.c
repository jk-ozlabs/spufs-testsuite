#define _ATFILE_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
	printf("%d\n", getpid());
	fflush(stdout);

	*(int *)NULL = 1;

	return EXIT_SUCCESS;
}

#include <test/utils.h>
#include <test/spu_syscalls.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "gang-presence.h"

/* create a gang, reference it with open(), then destroy the gang. */

int main(void)
{
	int gang, fd;
	char *gang_name;

	gang_name = name_spu_gang();
	gang = spu_create(gang_name, SPU_CREATE_GANG, 0700);
	assert(gang >= 0);

	/* create an extra reference to the gang */
	fd = open(gang_name, O_RDONLY);
	assert(fd >= 0);

	if (close(gang)) {
		perror("close(gang)");
		return EXIT_FAILURE;
	}

	if (check_gang_presence_on_close(gang_name, fd))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

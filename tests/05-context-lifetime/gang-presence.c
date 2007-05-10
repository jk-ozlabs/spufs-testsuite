
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* check that a gang exists before a fd is closed, and that it no longer
 * exists once fd is closed
 */
int check_gang_presence_on_close(const char *gang_name, int fd)
{
	struct stat statbuf;

	if (stat(gang_name, &statbuf)) {
		fprintf(stderr, "gang destroyed too early\n");
		return -1;
	}

	if (close(fd)) {
		perror("close(fd)");
		return -1;
	}

	if (!stat(gang_name, &statbuf)) {
		fprintf(stderr, "gang not destroyed\n");
		return -1;
	}

	if (errno != ENOENT) {
		fprintf(stderr, "Wrong errno (%d) for stat\n", errno);
		return -1;
	}

	return 0;
}

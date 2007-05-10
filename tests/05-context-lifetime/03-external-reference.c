#include <test/utils.h>
#include <test/spu_syscalls.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>

#include "gang-presence.h"

/* create a gang, reference it with open() from another process, then destroy
 * the gang.
 */

int main(void)
{
	int gang, fd, pid, status, sock[2];
	char *gang_name, buf = '\0';

	gang_name = name_spu_gang();
	gang = spu_create(gang_name, SPU_CREATE_GANG, 0700);
	assert(gang >= 0);

	/* a socket for parent<->child communication. Only used to synchronise
	 * the two processes. sock[0] is for the parent, sock[1] is for the
	 * child. */
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock)) {
		perror("pipe");
		return EXIT_FAILURE;
	}

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return EXIT_FAILURE;
	}

	if (pid > 0) {

		/* parent: wait for the child to open the gang dir */
		read(sock[0], &buf, 1);

		if (close(gang)) {
			perror("close(gang)");
			return EXIT_FAILURE;
		}

		/* tell the child that we've closed the gang */
		write(sock[0], &buf, 1);

		if (waitpid(pid, &status, 0) == -1) {
			perror("waitpid");
			return EXIT_FAILURE;
		}

		if (status)
			return EXIT_FAILURE;

	} else {
		if (close(gang)) {
			perror("close");
			return EXIT_FAILURE;
		}

		/* child: grab a reference to the gang */
		fd = open(gang_name, O_RDONLY);
		if (fd < 0) {
			perror("open");
			return EXIT_FAILURE;
		}

		/* tell the parent that we have a reference to the gang */
		write(sock[1], &buf, 1);

		/* wait for the parent to have derefed the gang */
		read(sock[1], &buf, 1);

		/* make sure the file descriptor table is sane by forking
		 * a new process that just returns success. We've had a bug
		 * where the kernel will panic on this fork. */
		pid = fork();
		if (pid < 0) {
			perror("fork");
			return EXIT_FAILURE;

		} else if (pid == 0) {
			return EXIT_SUCCESS;

		} else {
			if (waitpid(pid, &status, 0) == -1) {
				perror("waitpid");
				return EXIT_FAILURE;
			}
			if (status)
				return EXIT_FAILURE;
		}

		/* check that closing the fd destroys the gang */
		if (check_gang_presence_on_close(gang_name, fd))
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

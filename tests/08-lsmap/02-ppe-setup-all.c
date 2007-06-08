#define _ATFILE_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>


/*
 * test to set all of local store - fill LS with instructions:
 *
 * br	2
 * stop 0x0
 * ...
 * ...
 * nop
 * stop 0x1337
 *
 * So the pc skips over every odd instruction, and we should see a 0x1337 stop
 * code.
 */

int main(void)
{
	int ctx, ls_fd, rc, i;
	const char *name = "/spu/ppe-setup-all";
	uint32_t buf[2], entry, status;

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	ls_fd = openat(ctx, "mem", O_RDWR);
	if (ls_fd < 0) {
		perror("openat");
		return EXIT_FAILURE;
	}

	/* setup the skip-odd-instructions for all but the last two slots */
	buf[0] = 0x32000100; /* br   2		*/
	buf[1] = 0x00000000; /* stop 0x0	*/
	for (i = 0; i < LS_SIZE / sizeof(buf) - 1; i++) {
		rc = write(ls_fd, &buf, sizeof(buf));
		if (rc != sizeof(buf)) {
			perror("write");
			return EXIT_FAILURE;
		}
	}

	/* add the final nop-and-stop at the end of LS */
	buf[0] = 0x40200000; /* nop		*/
	buf[1] = 0x00001337; /* stop 0x1337	*/
	rc = write(ls_fd, &buf, sizeof(buf));
	if (rc != sizeof(buf)) {
		perror("write");
		return EXIT_FAILURE;
	}

	entry = 0;
	rc = spu_run(ctx, &entry, &status);

	if (rc < 0) {
		perror("spu_run");
		return EXIT_FAILURE;
	}

	if (!(rc & 0x2)) {
		fprintf(stderr, "spu didn't stop-and-signal?\n");
		return EXIT_FAILURE;
	}

	if (rc >> 16 != 0x1337) {
		fprintf(stderr, "spu executed different stop instruction?\n");
		return EXIT_FAILURE;
	}

	printf("npc: 0x%x, status: 0x%x\n", entry, rc);

	return EXIT_SUCCESS;
}

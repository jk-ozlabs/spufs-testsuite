#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>

/*
 * test that the entry point advances by 4 for each spu_run, and that it wraps
 * once we reach the end of local store
 */

int main(void)
{
	int rc, ctx;
	const char *name = "/spu/" __FILE__;
	uint32_t entry;

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	entry = 0;
	do {
		int expected_entry = (entry + 4) % LS_SIZE;
		rc = spu_run(ctx, &entry, NULL);

		if (rc < 0) {
			perror("spu_run");
			return EXIT_FAILURE;
		}

		if (entry != expected_entry) {
			fprintf(stderr, "entry = 0x%x, expected 0x%x\n",
					entry, expected_entry);
			return EXIT_FAILURE;
		}
	} while (entry != 0);

	return EXIT_SUCCESS;
}

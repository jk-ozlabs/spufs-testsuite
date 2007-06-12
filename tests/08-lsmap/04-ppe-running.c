#define _ATFILE_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>


/**
 * Test both reeading and writing of local store while a SPE is running.
 *
 * Initialise the first quadword in local store to zero, and upload SPU
 * code to set it to all ones, and loop while it's not zero.
 *
 * The PPE side checks for the all-ones pattern, then sets the quadword to
 * zero.
 */

void *ppe_thread(void *data)
{
	volatile uint32_t *ls = data;
	int i = 0;

	/* wait for the SPE app to set the first quadword of local store to all
	 * ones */
	while ((ls[0] & ls[1] & ls[2] & ls[3]) != 0xffffffff)
		i++;

	/* set the first quadword to all zeros */
	ls[0] = ls[1] = ls[2] = ls[3] = 0x00000000;

	printf("%s: %d loops\n", __func__, i);

	return NULL;
}

int main(void)
{
	int ctx, ls_fd, rc;
	const char *name = "/spu/ppe-running";
	uint32_t *ls_map, entry, status;
	pthread_t thread;

	ctx = spu_create(name, 0, 0755);
	assert(ctx >= 0);

	ls_fd = openat(ctx, "mem", O_RDWR);
	if (ls_fd < 0) {
		perror("openat: mem");
		return EXIT_FAILURE;
	}

	ls_map = mmap(NULL, LS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			ls_fd, 0);
	if (ls_map == MAP_FAILED) {
		perror("mmap");
		return EXIT_FAILURE;
	}

	ls_map[0] = 0x00000000;
	ls_map[1] = 0x00000000;
	ls_map[2] = 0x00000000;
	ls_map[3] = 0x00000000;

	ls_map[4] = 0x40ffff83; /* il      $3,-1	*/
	ls_map[5] = 0x20800003; /* stqa    $3,0		*/
	ls_map[6] = 0x30800003; /* lqa     $3,0		*/
	ls_map[7] = 0x217fff83; /* brnz    $3,(. - 4)	*/
	ls_map[8] = 0x00001337; /* stop    0x1337	*/

	rc = pthread_create(&thread, NULL, ppe_thread, ls_map);
	if (rc) {
		perror("pthread_create");
		return EXIT_FAILURE;
	}

	/* give the ppe thread time to schedule */
	usleep(1000);

	entry = 0x10;
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

	if (pthread_join(thread, NULL)) {
		perror("pthread_join");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

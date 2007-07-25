#define _ATFILE_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>

/**
 * Send a number of (monotonically increasing) messages to a SPE, and ensure
 * that all messages are received, and in the right order
 */

#define nr_msgs	(1024000)

struct spe_thread_info {
	pthread_t	pthread;
	int		ctx;
	int		run_rc;
	uint32_t	entry;
};
static int thread_abort;

static void *spe_thread(void *data)
{
	struct spe_thread_info *thread = data;

	thread->run_rc = spu_run(thread->ctx, &thread->entry, NULL);
	if (thread->run_rc < 0)
		perror("spu_run");

	thread_abort = 1;

	return NULL;
}

static uint32_t spe_program[] = {
	/* set r2 to the stop message (0xffffffff 00000000 00000000 00000000) */
	0x32f80002, /* fsmbi   $2,0xf000    */

	/* set r0 to 0 */
	0x40800000, /* il      $0,0         */

	/* read a message into r1 */
	0x01a00e81, /* rdch    $1,$ch29     */

	/* compare the message to the stop message. If they're equal, stop */
	0x78008083, /* ceq     $3,$1,$2     */
	0x20000103, /* brz     $3,0x18 # 18 */
	0x00001337, /* stop    0x1337       */

	/* compare to the message count (and increment). If they're equal,
	 * branch back up */
	0x78000083, /* ceq     $3,$1,$0     */
	0x1c004000, /* ai      $0,$0,1      */
	0x217ffd03, /* brnz    $3,0xc       */
	0x00000000, /* stop                 */
};

int main()
{
	int ls_fd, mbox_fd, rc, i;
	uint32_t msg;
	struct spe_thread_info thread;
	char *name = "/spu/02-mbox-multiple";

	thread.ctx = spu_create(name, 0, 0755);
	assert(thread.ctx >= 0);

	ls_fd = openat(thread.ctx, "mem", O_RDWR);
	assert(ls_fd >= 0);

	mbox_fd = openat(thread.ctx, "wbox", O_WRONLY);
	if (mbox_fd <= 0) {
		perror("open:wbox");
		return EXIT_FAILURE;
	}

	rc = write(ls_fd, spe_program, sizeof(spe_program));
	assert(rc = sizeof(spe_program));

	thread.entry = 0;

	rc = pthread_create(&thread.pthread, NULL, spe_thread, &thread);
	assert(rc == 0);

	for (i = 0; i < nr_msgs; i++) {
		msg = i;

		rc = write(mbox_fd, &msg, sizeof(msg));
		if (rc != sizeof(msg)) {
			perror("write");
			return EXIT_FAILURE;
		}

		if (thread_abort)
			break;
	}

	/* send the stop message */
	msg = 0xffffffff;
	rc = write(mbox_fd, &msg, sizeof(msg));
	if (rc != sizeof(msg)) {
		perror("write");
		return EXIT_FAILURE;
	}

	rc = pthread_join(thread.pthread, NULL);


	if (thread.run_rc != 0x13370002) {
		fprintf(stderr, "Incorrect stop code: 0x%x\n", thread.run_rc);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

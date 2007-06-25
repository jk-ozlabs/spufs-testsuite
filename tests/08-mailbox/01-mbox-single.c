#define _ATFILE_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>

struct spe_thread_info {
	pthread_t	pthread;
	int		ctx;
	int		run_rc;
	uint32_t	entry;
};

static void *spe_thread(void *data)
{
	struct spe_thread_info *thread = data;

	thread->run_rc = spu_run(thread->ctx, &thread->entry, NULL);
	if (thread->run_rc < 0)
		perror("spu_run");

	return NULL;
}

static uint32_t spe_program[] = {
	0x01a00e80, /* rdch r0,ch29 */
	0x00001337, /* stop 0x1337  */
};

int main()
{
	int ls_fd, mbox_fd, rc;
	uint32_t msg;
	struct spe_thread_info thread;
	char *name = "/spu/ctx-" __FILE__;

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

	msg = 3;
	rc = write(mbox_fd, &msg, sizeof(msg));
	if (rc < 0) {
		perror("write");
		return EXIT_FAILURE;
	}

	if (rc != sizeof(msg)) {
		fprintf(stderr, "short write to wbox? sent %d, expected %d\n",
				rc, sizeof(msg));
		return EXIT_FAILURE;
	}

	rc = pthread_join(thread.pthread, NULL);
	assert(rc == 0);

	if (thread.run_rc != 0x13370002) {
		fprintf(stderr, "Incorrect stop code: 0x%x\n", thread.run_rc);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

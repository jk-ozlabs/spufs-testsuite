#define _ATFILE_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>

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
/*
	. = 0x10
	il	r0,1
	stqa	r0,0x0
load:
	lqa	r1,0x0
	ceq	r2,r1,r0
	brnz	r2,load
	stop	0x1337
*/
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x40800080,
	0x20800000,
	0x30800001,
	0x78000082,
	0x217fff02,
	0x00001337,
};

int main()
{
	int ls_fd, reg_fd, rc;
	uint32_t *ls_map, reg;
	struct spe_thread_info thread;
	char *name = "/spu/ctx-" __FILE__;

	thread.ctx = spu_create(name, 0, 0755);
	assert(thread.ctx >= 0);

	ls_fd = openat(thread.ctx, "mem", O_RDWR);
	assert(ls_fd >= 0);

	ls_map = mmap(NULL, LS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			ls_fd, 0);
	assert(ls_map != MAP_FAILED);

	reg_fd = openat(thread.ctx, "regs", O_RDWR);
	assert(reg_fd >= 0);

	memcpy(ls_map, spe_program, sizeof(spe_program));

	thread.entry = 0x10;

	rc = pthread_create(&thread.pthread, NULL, spe_thread, &thread);
	assert(rc == 0);

	/* wait for ls address 0x0 to become 1 */
	while (!thread_abort && ls_map[0] == 0)
		usleep(1);

	/* cause spe to be scheduled out */
	rc = read(reg_fd, &reg, sizeof(reg));
	if (rc != sizeof(reg)) {
		perror("read");
		return EXIT_FAILURE;
	}

	printf("SPE r0: 0x%08x\n", reg);
	if (reg != 0x1) {
		printf("Unexpected value of r0!\n");
		return EXIT_FAILURE;
	}

	/* set ls address 0x0 to 0 */
	ls_map[0] = 0;

	rc = pthread_join(thread.pthread, NULL);

	if (thread.run_rc != 0x13370002) {
		fprintf(stderr, "Incorrect stop code: 0x%x\n", thread.run_rc);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

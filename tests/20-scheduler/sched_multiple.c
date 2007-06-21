
#define _ATFILE_SOURCE

#include <test/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include <test/spu_syscalls.h>
#include <test/hw.h>

static void usage(char *progname)
{
	fprintf(stderr, "Usage: %s <file>\n", progname);
}

static int read_dataset(const char *filename, uint8_t *n_spes, int *n_writes,
		uint8_t **write_pattern)
{
	void *buf;
	int fd, len;
	struct stat statbuf;
	struct {
		uint8_t n_spes;
		uint8_t pattern[0];
	} *data;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return -1;
	}

	if (fstat(fd, &statbuf)) {
		perror("fstat");
		return -1;
	}

	len = statbuf.st_size;

	buf = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
	if (buf == MAP_FAILED) {
		perror("mmap");
		return -1;
	}

	data = buf;

	*n_spes = data->n_spes;
	*write_pattern = data->pattern;
	*n_writes = (len - sizeof(data->n_spes));

	return 0;
}

struct spe_thread_info {
	int		ctx, mbox;
	pthread_t	pthread;
	int		run_rc;
	int		writes;
};

static int thread_abort;

static void *spe_thread(void *data)
{
	struct spe_thread_info *thread = data;
	uint32_t entry = 0x10;

	thread->run_rc = spu_run(thread->ctx, &entry, NULL);

	if (thread->run_rc != 0x13370002) {
		thread_abort = 1;
		fprintf(stderr, "thread exited with invalid rc: 0x%x\n",
					thread->run_rc);
	}

	return NULL;
}

static uint32_t spe_program[] = {
	/* use the first quadword as a counter */
	0x0000000,
	0x0000000,
	0x0000000,
	0x0000000,

	/* load 0x0 into r7 */
	0x40800007,	/* il	r7, 0			*/

	/* load 0xffffffff 00000000 00000000 00000000 into r5 */
	0x32f80005,	/* fsmbi r5,0xf000		*/

	/* read our id into r3 */
	0x01a00e83,	/* rdch  r3,ch29		*/

	/* read a message into r4 */
	0x01a00e84,	/* rdch  r4,ch29		*/

	/* increment counter and store at 0x0 */
	0x1c004387,	/* ai    r7,r7,1		*/
	0x20800004,	/* stqa  r7,0			*/

	/* check if this is a stop message. if so, stop */
	0x78010286,	/* ceq   r6,r5,r4		*/
	0x20000106,	/* brz   r6,.+2			*/
	0x00001337,	/* stop  0x1337			*/

	/* check if this is our id. if not, don't loop, and error out */
	0x78010186,	/* ceq   r6,r3,r4		*/
	0x217ffc86,	/* brnz  r6,0x10		*/
	0x00000000,	/* stop  0x00			*/
};


int main(int argc, char **argv)
{
	int i, pattern_len;
	uint8_t n_spes, *pattern;
	struct spe_thread_info *threads;

	if (argc != 2) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	if (read_dataset(argv[1], &n_spes, &pattern_len, &pattern))
		return EXIT_FAILURE;

	printf("using %d spe contexts, for %d ops\n", n_spes, pattern_len);

	threads = calloc(n_spes, sizeof(*threads));

	/* set up the contexts */
	for (i = 0; i < n_spes; i++) {
		char *name = name_spu_context(NULL);
		int fd, rc;

		threads[i].ctx = spu_create(name, 0, 0755);
		assert(threads[i].ctx >= 0);

		fd = openat(threads[i].ctx, "mem", O_RDWR);
		assert(fd >= 0);
		threads[i].writes = 0;

		threads[i].mbox = openat(threads[i].ctx, "wbox", O_WRONLY);
		if (threads[i].mbox < 0) {
			perror("openat");
		}
		assert(threads[i].mbox >= 0);

		rc = write(fd, spe_program, sizeof(spe_program));
		if (rc != sizeof(spe_program)) {
			perror("write:mem");
			return EXIT_FAILURE;
		}

		if (pthread_create(&threads[i].pthread, NULL, spe_thread,
					&threads[i])) {
			perror("pthread_create");
			return EXIT_FAILURE;
		}
	}

	for (i = 0; i < pattern_len; i++) {
		uint32_t buf = pattern[i];
		struct spe_thread_info *thread = threads + pattern[i];
		thread->writes++;
		printf("[%d] sending to %d\n", i, pattern[i]);
		if (write(thread->mbox, &buf, sizeof(buf)) != sizeof(buf)) {
			perror("write:mbox");
			return EXIT_FAILURE;
		}
		if (thread_abort)
			return EXIT_FAILURE;
	}

	/* send completion, wait for all threads to complete */
	for (i = 0; i < n_spes; i++) {
		uint32_t buf = 0xffffffff;
		printf("[%d] %d writes\n", i, threads[i].writes);
		if (write(threads[i].mbox, &buf, sizeof(buf)) != sizeof(buf)) {
			perror("write:mbox");
			return EXIT_FAILURE;
		}

		if (pthread_join(threads[i].pthread, NULL)) {
			perror("pthread_join");
			return EXIT_FAILURE;
		}

		printf("thread %2d ended, status=0x%08x\n",
				i, threads[i].run_rc);

		/* check for stop-and-signal status, with 0x1337 stop code */
		if (threads[i].run_rc != 0x13370002) {
			fprintf(stderr, "thread exited with invalid rc: 0x%x\n",
					threads[i].run_rc);
			return EXIT_FAILURE;
		}
	}


	return EXIT_SUCCESS;
}

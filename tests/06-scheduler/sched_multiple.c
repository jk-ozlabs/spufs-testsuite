
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>

#include <libspe.h>

extern spe_program_handle_t sched_multiple_spe_app;

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

int main(int argc, char **argv)
{
	int i, n_groups, pattern_len;
	uint8_t n_spes, *pattern;
	struct sched_param sched_param;
	spe_gid_t *groups;
	speid_t *threads;

	if (argc != 2) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	if (read_dataset(argv[1], &n_spes, &pattern_len, &pattern))
		return EXIT_FAILURE;

	n_groups = (n_spes - 1) / MAX_THREADS_PER_GROUP + 1;
	printf("using %d spe contexts in %d groups, for %d ops\n",
			n_spes, n_groups, pattern_len);

	sched_param.sched_priority = 50;
	if (sched_setscheduler(0, SCHED_RR, &sched_param)) {
		perror("sched_setscheduler");
		return EXIT_FAILURE;
	}

	groups = malloc(n_groups * sizeof(*groups));
	for (i = 0; i < n_groups; i++) {
		groups[i] = spe_create_group(SCHED_RR, 50, 1);
		if (!groups[i]) {
			perror("spe_create_group");
			return EXIT_FAILURE;
		}
	}

	threads = malloc(n_spes * sizeof(*threads));

	for (i = 0; i < n_spes; i++) {
		threads[i] = spe_create_thread(groups[i/16],
				&sched_multiple_spe_app, NULL, NULL, -1, 0);
		if (!threads[i]) {
			perror("spe_create_thread");
			return EXIT_FAILURE;
		}
	}

	for (i = 0; i < pattern_len; i++) {
		speid_t thread = threads[pattern[i]];
		printf("[%d] sending to %d\n", i, pattern[i]);
		spe_write_in_mbox(thread, pattern[i]);
	}

	return EXIT_SUCCESS;
}

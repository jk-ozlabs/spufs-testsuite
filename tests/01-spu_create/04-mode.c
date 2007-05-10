#include <test/utils.h>
#include <test/spu_syscalls.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#define TEST_MODES (	S_IRUSR | S_IWUSR | S_IXUSR | \
			S_IRGRP | S_IWGRP | S_IXGRP | \
			S_IROTH | S_IWOTH | S_IXOTH )

int main(void)
{
	int ctx, mode;
	char *name;
	struct stat stat;

	printf("%s: checking modes: 0%04o\n", __FILE__, TEST_MODES);
	fflush(stdout);

	umask(0);

	for (mode = 0; mode <= TEST_MODES; mode++) {
		if ((mode & TEST_MODES) != mode)
			continue;

		name = name_spu_context(NULL);

		ctx = spu_create(name, 0, mode);
		assert(ctx);

		if (fstat(ctx, &stat)) {
			perror("fstat");
			return EXIT_FAILURE;
		}

		stat.st_mode &= TEST_MODES;

		if (stat.st_mode != mode) {
			fprintf(stderr, "incorrect mode 0%04o, "
					"expecting 0%04o\n",
					stat.st_mode, mode);
			return EXIT_FAILURE;
		}

		close(ctx);
		free(name);
	}


	return EXIT_SUCCESS;
}

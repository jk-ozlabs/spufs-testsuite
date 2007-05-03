#include <test/utils.h>
#include <test/spu_syscalls.h>
#include <stdlib.h>
#include <errno.h>

const unsigned long spu_create_all_flags =
	SPU_CREATE_EVENTS_ENABLED |
	SPU_CREATE_GANG |
	SPU_CREATE_NOSCHED |
	SPU_CREATE_ISOLATE |
#if 0
	SPU_CREATE_AFFINITY_SPU 
	SPU_CREATE_AFFINITY_MEM
#endif
	0;

int main(void)
{
	char *name = name_spu_context(NULL);
	int bit, ctx;

	for (bit = 0; bit < 32; bit++) {
		unsigned long flags = 1ul << bit;
		if (flags & spu_create_all_flags)
			continue;
		ctx = spu_create(name, flags, 0);
		if (ctx != -1) {
			fprintf(stderr, "spu_create() accepted invalid flags: "
					"0x%08lx\n", flags);
			return EXIT_FAILURE;
		}
		if (errno != EINVAL) {
			fprintf(stderr, "spu_create(): wrong errno (%d) with "
					"invalid flags: 0x%08lx\n",
					errno, flags);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

#include <libspe2.h>
#include <stdlib.h>
#include <unistd.h>

/* create a context, then destroy it */

int main(void)
{
	spe_context_ptr_t ctx;
	int rc;

	ctx = spe_context_create(0, NULL);

	if (!ctx) {
		perror("spe_context_create");
		return EXIT_FAILURE;
	}

	rc = spe_context_destroy(ctx);

	if (rc) {
		perror("spe_context_destroy");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

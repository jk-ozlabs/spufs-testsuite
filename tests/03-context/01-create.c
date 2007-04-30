
#include <libspe2.h>
#include <stdlib.h>
#include <unistd.h>

/* create a context */

int main(void)
{
	spe_context_ptr_t ctx;

	ctx = spe_context_create(0, NULL);

	if (!ctx) {
		perror("spe_context_create");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

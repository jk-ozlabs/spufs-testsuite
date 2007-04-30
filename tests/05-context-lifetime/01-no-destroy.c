#include <libspe2.h>
#include <stdlib.h>
#include <unistd.h>

/* create a gang with a context, then destroy the gang */

int main(void)
{
	spe_gang_context_ptr_t gang;
	spe_context_ptr_t ctx;

	gang = spe_gang_context_create(0);

	if (!gang) {
		perror("spe_gang_context_create");
		return EXIT_FAILURE;
	}

	ctx = spe_context_create(0, gang);

	if (!ctx) {
		perror("spe_context_create");
		return EXIT_FAILURE;
	}

	if (spe_gang_context_destroy(gang)) {
		perror("spe_gang_context_destroy");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

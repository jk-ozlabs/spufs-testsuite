#include <libspe2.h>
#include <stdlib.h>
#include <unistd.h>

/* create a gang, then destroy it */

int main(void)
{
	spe_gang_context_ptr_t gang;
	int rc;

	gang = spe_gang_context_create(0);

	if (!gang) {
		perror("spe_gang_context_create");
		return EXIT_FAILURE;
	}

	rc = spe_gang_context_destroy(gang);

	if (rc) {
		perror("spe_gang_context_destroy");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

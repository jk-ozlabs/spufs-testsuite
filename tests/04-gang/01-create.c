#include <libspe2.h>
#include <stdlib.h>
#include <unistd.h>

/* create a gang */

int main(void)
{
	spe_gang_context_ptr_t gang;

	gang = spe_gang_context_create(0);

	if (!gang) {
		perror("spe_gang_context_create");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


#include <spu_intrinsics.h>
#include <stdio.h>

int main(void)
{
	unsigned int id, data;

	id = spu_readch(SPU_RdInMbox);

	for (;;) {
		data = spu_readch(SPU_RdInMbox);
		if (data != id) {
			printf("wrong mailbox? %u != %d", data, id);
			break;
		}
	}

	return -1;
}


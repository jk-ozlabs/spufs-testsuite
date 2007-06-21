#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <assert.h>

#include "test.h"
#define ROLE "slave "

volatile int buffer[BUFFER_SIZE] __attribute__ ((aligned (16)));

int main (int spuid, unsigned long long addr, void *envp)
{
	int i;
	DBG("about to report in\n");

	spu_writech(SPU_WrOutMbox, 1);
	DBG("reported in\n");

	DBG("waiting for buffer mark instruction\n");

	/* If we're being read from, fill up the buffer with the sig */
	if (spu_readch(SPU_RdInMbox)) {
		DBG("marking buffer\n");
		for (i = 0; i < BUFFER_SIZE; i++)
			buffer[i] = BUFFER_SIGNATURE;
	} else
		DBG("NOT marking buffer\n");

	/* Write our buffer address to the PPE. */
	spu_writech(SPU_WrOutMbox, (unsigned int)buffer);

	DBG("waiting for termination signal\n");
	/* Wait for a write from the PPE before exiting. */
	spu_readch(SPU_RdInMbox);

	assert(buffer[BUFFER_SIZE - 1] = BUFFER_SIGNATURE);
	
	return 0;
}

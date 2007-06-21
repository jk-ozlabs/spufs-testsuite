#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <assert.h>

#include "test.h"
#ifdef DIRECTION_PUT
#define ROLE	"writer"
#else
#define ROLE	"reader"
#endif

static inline void dma_wait(int dma_tag)
{
	spu_writech(MFC_WrTagUpdate, 0);

	while (spu_readchcnt(MFC_RdTagStat) == 0);

	spu_readch(MFC_RdTagStat);
	spu_writech(MFC_WrTagMask, ( 1 << dma_tag ));
	spu_writech(MFC_WrTagUpdate, 2 /* Update all */);
	spu_readch(MFC_RdTagStat);
}

volatile int buffer[BUFFER_SIZE] __attribute__ ((aligned (16)));

int main (int spuid, unsigned long long addr, void *envp)
{
	unsigned int start, end, ls_addr;
	int j;

	spu_writech(SPU_WrOutMbox, 1);
	DBG("reported in\n");

	DBG("waiting for local store address\n");
	ls_addr = spu_readch(SPU_RdInMbox);
	DBG("got local store address %x\n", ls_addr);

	/* Setup the decrementer */
	spu_writech (SPU_WrEventMask, 0);
	spu_writech (SPU_WrEventAck, 32);
	spu_writech (SPU_WrDec, 0xFFFFFFFFUL);
	start = spu_readch(SPU_RdDec);

	for (j = 0; j < NUM_ITERS; j++) {
#ifdef DIRECTION_PUT
		spu_mfcdma32(buffer, ls_addr, BUFFER_BYTES, 0, MFC_PUT_CMD);
		dma_wait(0);
#else
		spu_mfcdma32(buffer, ls_addr, BUFFER_BYTES, 0, MFC_GET_CMD);
		dma_wait(0);
		assert(buffer[BUFFER_SIZE - 1] == BUFFER_SIGNATURE);
#endif
	}

	end = spu_readch(SPU_RdDec);
	DBG("dma completed\n");
	spu_writech(SPU_WrOutMbox, start - end);

	return 0;
}

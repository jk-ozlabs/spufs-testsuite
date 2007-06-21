#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <libspe.h>
#include "test.h"
#define ROLE "master"

#ifdef DIRECTION_PUT
#define CHILD_BINARY	dma_put_spe_app
#define CHILD_NAME	"writer"
#define SLAVE_MARK	0
#else
#define CHILD_BINARY	dma_get_spe_app
#define CHILD_NAME	"reader"
#define SLAVE_MARK	1
#endif

#define NUM_THREADS	8

extern spe_program_handle_t slave_spe_app, CHILD_BINARY;

#undef DBG
#define DBG(x...)	printf(ROLE ": " x)

int main(int argc, char* argv[])
{
	speid_t  slave_tids[NUM_THREADS], child_tids[NUM_THREADS];
	int status, slave_addr, i;
	unsigned int ticks;
	void *ls;

	for (i = 0; i < NUM_THREADS; i++) {
		slave_tids[i] = spe_create_thread(0, &slave_spe_app, NULL, NULL,
					      0x1, 0);
		assert(slave_tids[i]);

		DBG("waiting for slave to report in\n");
		while (!spe_stat_out_mbox(slave_tids[i]));
		spe_read_out_mbox(slave_tids[i]);

		ls = spe_get_ls(slave_tids[i]);

		DBG("telling slave %sto mark buffer\n",
			SLAVE_MARK ? "not " : "");
		spe_write_in_mbox(slave_tids[i], SLAVE_MARK);

		/* Read the buffer address from the slave */
		DBG("reading buffer address from slave\n");
		while (!spe_stat_out_mbox(slave_tids[i]));
		slave_addr = spe_read_out_mbox(slave_tids[i]);
		DBG("got slave addr %x + ls %p\n", slave_addr, ls);

		child_tids[i] = spe_create_thread(0, &CHILD_BINARY, NULL, NULL,
					      0x10, 0);
		assert(child_tids[i]);

		DBG("waiting for " CHILD_NAME " to report in\n");
		while (!spe_stat_out_mbox(child_tids[i]));
		spe_read_out_mbox(child_tids[i]);

		/* Write the local store address to the child */
		DBG("sending buffer addr to " CHILD_NAME "\n");
		spe_write_in_mbox(child_tids[i], (unsigned int)ls + slave_addr);
	}

	for (i = 0; i < NUM_THREADS; i++) {
		/* Read ticks back from the child */
		DBG("waiting for tick count from " CHILD_NAME "\n");
		while (!spe_stat_out_mbox(child_tids[i]));
		ticks = spe_read_out_mbox(child_tids[i]);
		printf("%8.2f\n", decr_to_bw(ticks));

		/* Tell the slave to exit. */
		spe_write_in_mbox(slave_tids[i], 1);

		spe_wait(slave_tids[i], &status, 0);
		DBG("slave returned status: %04x\n", status);

		spe_wait(child_tids[i], &status, 0);
		DBG(CHILD_NAME " returned status: %04x\n", status);
	}

	return 0;
}

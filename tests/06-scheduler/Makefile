
CFLAGS += $(LIBSPE1_CFLAGS)
LDFLAGS += $(LIBSPE1_LDFLAGS)

tests += sched_multiple

sched_multiple: sched_multiple.o sched_multiple_spe_embed.o

sched_multiple_spe_embed.o: sched_multiple_spe
	embedspu $^_app $^ $@

sched_multiple_spe: CC=spu-gcc
sched_multiple_spe: LDFLAGS=


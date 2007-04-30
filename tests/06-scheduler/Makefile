
CFLAGS += $(LIBSPE1_CFLAGS)
LDFLAGS += $(LIBSPE1_LDFLAGS)

obj += sched_multiple

sched_multiple: sched_multiple.o sched_multiple_spe_embed.o

sched_multiple_spe: CC=$(SPU_CC)
sched_multiple_spe: LDFLAGS=


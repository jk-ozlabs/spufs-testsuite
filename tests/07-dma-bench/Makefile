CFLAGS += $(LIBSPE1_CFLAGS)
LDFLAGS += $(LIBSPE1_LDFLAGS)

obj += test_dma_put test_dma_get

test_dma_put: CPPFLAGS+=-DDIRECTION_PUT
test_dma_get: CPPFLAGS+=-DDIRECTION_GET

test_dma_put: test_dma_put.o dma_put.spe-embed.o slave.spe-embed.o
test_dma_get: test_dma_get.o dma_get.spe-embed.o slave.spe-embed.o

test_dma_%.o: test_dma.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

dma_%.spe: spe.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

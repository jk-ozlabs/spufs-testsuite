
dirs = 00-testsuite 01-spu_create 02-spu_run 03-context 04-gang \
	05-context-lifetime 06-lsmap 07-reg-file 08-mailbox 11-nosched \
	20-scheduler 30-dma-bench

all: $(dirs)

$(dirs) : % : %/Makefile
	cd $@ && make -f ../Makefile.lib all

clean:
	for d in $(dirs); do (cd $$d && make -f ../Makefile.lib clean); done

.PHONY: all clean $(dirs)


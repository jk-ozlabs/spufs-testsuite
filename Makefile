
all:
	cd tests && make all

clean:
	cd tests && make clean

check: clean all
	./run-tests

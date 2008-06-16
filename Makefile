# Testsuite for the Linux SPU filesystem
#
# Copyright (C) IBM Corporation, 2007
#
# Author: Jeremy Kerr <jk@ozlabs.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

CC=gcc
CFLAGS=-Wall -Werror -g -O2 -Ilib/include
LDFLAGS=

MAKEFLAGS=--no-print-directory

all: tests benchmarks

.PHONY: tests benchmarks libs

tests benchmarks: bin/run-tests lib/slowfs/slowfs
	$(MAKE) -C $@ all


bin/run-tests: lib/talloc/talloc.o bin/run-tests.o bin/capabilities.o bin/test.o
	$(LINK.o) -o $@ $^

libs: lib/talloc/talloc.o

lib/slowfs/slowfs: CFLAGS += $(shell pkg-config fuse --cflags)
lib/slowfs/slowfs: LDFLAGS += $(shell pkg-config fuse --libs)

clean:
	cd tests && make clean
	cd benchmarks && make clean
	rm -f bin/capabilities bin/run-tests
	rm -f bin/*.o lib/talloc/talloc.o

check: clean all
	bin/run-tests

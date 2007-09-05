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

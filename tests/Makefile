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

dirs = 00-testsuite 01-spu_create 02-spu_run 03-context 04-gang \
	05-context-lifetime 06-lsmap 07-reg-file 08-mailbox 09-syscalls \
	10-coredump 11-nosched 12-affinity 14-dma 20-scheduler 30-dma-bench

all: $(dirs)

$(dirs) : % : %/Makefile
	cd $@ && make -f ../Makefile.lib all

clean:
	for d in $(dirs); do (cd $$d && make -f ../Makefile.lib clean); done

.PHONY: all clean $(dirs)


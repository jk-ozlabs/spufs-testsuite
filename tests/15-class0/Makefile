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

obj += 01-dma-alignment-signal 02-dma-alignment-event \
       03-invalid-dma-signal 04-invalid-dma-event \
       05-illegal-insn-signal 06-illegal-insn-event


01-dma-alignment-signal: 01-dma-alignment-signal.o class0-apps.o
02-dma-alignment-event:  02-dma-alignment-event.o class0-apps.o
03-invalid-dma-signal:   03-invalid-dma-signal.o class0-apps.o
04-invalid-dma-event:    04-invalid-dma-event.o class0-apps.o
05-illegal-insn-signal:  05-illegal-insn-signal.o class0-apps.o
06-illegal-insn-event:   06-illegal-insn-event.o class0-apps.o


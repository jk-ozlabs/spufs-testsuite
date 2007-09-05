#!/usr/bin/env python
#
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

import sys
from struct import pack
from random import random

try:
	out_file = file(sys.argv[1], 'wb')
	n_spes = int(sys.argv[2])
	n_writes = int(sys.argv[3])
except Exception, ex:
	print "Usage: %s <datafile> <n_spes> <n_messages>" % sys.argv[0]
	sys.exit(1)

out_file.write(pack('B', n_spes))
for i in range(0, n_writes):
	out_file.write(pack('B', int(random() * n_spes)))



#!/usr/bin/env python

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



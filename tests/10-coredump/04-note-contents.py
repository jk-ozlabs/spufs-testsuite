#!/usr/bin/python
#
# Testsuite for the Linux SPU filesystem
#
# Copyright (C) IBM Corporation, 2007
#
# Author: Michael Ellerman <michael@ellerman.id.au>
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

required = [
    'regs',
    'fpcr',
    'lslr',
    'decr',
    'decr_status',
    'mem',
    'signal1',
    'signal1_type',
    'signal2',
    'signal2_type',
    'event_mask',
    'event_status',
    'mbox_info',
    'ibox_info',
    'wbox_info',
    'dma_info',
    'proxydma_info',
    'object-id',
    'npc',
]

import os, sys, resource, atexit

def main():
    resource.setrlimit(resource.RLIMIT_CORE, (-1, -1))

    pipe = os.popen('spu-coredump')
    pid = pipe.read().strip()
    pipe.close()

    corefile = "core.%s" % pid

    if not os.path.exists(corefile):
        print >>sys.stderr, "Can't find corefile? ulimit? core_pattern?"
        return 2

    atexit.register(os.unlink, corefile)

    pipe = os.popen('spu-readelf -n %s' % corefile)

    found = {}
    for line in pipe.readlines():
        tokens = line.split()
        if len(tokens) == 0 or tokens[0] != 'SPU':
            continue

        fd, attr = tokens[2].split('/')
        d = found.get(fd, {})
        d[attr] = True
        found[fd] = d

    pipe.close()

    for fd in found.keys():
        d = found[fd]
        for attr in required:
            if not d.get(attr, False):
                print >>sys.stderr, \
                      "Required attribute %s not found in coredump!" % attr
                return 3

    return 0

sys.exit(main())

#!/usr/bin/python

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

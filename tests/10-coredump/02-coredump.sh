#!/bin/bash

ulimit -c unlimited

pid=$(spu-coredump)

corefile="core.$pid"

if [[ ! -f $corefile ]]; then
	echo "Can't find corefile? ulimit? core_pattern?" >&2
	exit 2
fi

readnotes $corefile

if [[ $? -ne 0 ]]; then
	exit 1
fi

rm -f $corefile

exit 0

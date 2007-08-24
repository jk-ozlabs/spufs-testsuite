#!/bin/bash

ulimit -c unlimited

pid=$(spu-coredump)

corefile="core.$pid"

if [[ ! -f $corefile ]]; then
	echo "Can't find corefile? ulimit? core_pattern?" >&2
	exit 2
fi

load_addr=$(readelf -l $corefile | awk '/LOAD/ {print $2; exit}')
load_addr=$(($load_addr))

dd if=$corefile of=out.bin bs=1 skip=$load_addr count=4
echo -en "\x7fELF" | cmp out.bin

if [[ $? -ne 0 ]]; then
	exit 1
fi

rm -f $corefile out.bin

exit 0

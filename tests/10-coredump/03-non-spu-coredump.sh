#!/bin/bash

ulimit -c unlimited

corelist=""

function get_note_size
{
	pid=$(coredump)

	corefile="core.$pid"

	if [[ ! -f $corefile ]]; then
		echo "Can't find corefile? ulimit? core_pattern?" >&2
		exit 2
	fi

	corelist="$corelist $corefile"

	size=$(readelf -n $corefile |awk '/Notes at offset/ {print $7}')
	size=${size%:}

	echo $size
}

function put_back_spufs
{
	modprobe spufs
	mount spufs
}

before_size=$(get_note_size)

trap 'put_back_spufs' EXIT

umount spufs || exit 1
modprobe -r spufs || exit 1

after_size=$(get_note_size)

if [[ $after_size != $before_size ]]; then
	echo "Error: Note size doesn't match w/wout spufs ($before_size != $after_size)!" >&2
	exit 1
fi

if [[ ! -z $corelist ]]; then
	rm -f $corelist
fi

exit 0

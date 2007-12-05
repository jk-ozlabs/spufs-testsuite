#!/bin/bash
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

ulimit -c unlimited
PATH=$PATH:/usr/sbin:/sbin

corelist=""

function get_note_size
{
	pid=$(./coredump)

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

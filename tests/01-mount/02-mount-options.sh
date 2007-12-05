#!/bin/bash
#
# Testsuite for the Linux SPU filesystem
#
#  Copyright (C) 2007 Sony Computer Entertainment Inc.
#  Copyright 2007 Sony Corporation
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; version 2 of the License.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

MOUNTDIR=./mounttest


if [ `id -u ` != 0 ] ; then
    echo "This test requires root permission." >&2
    exit 1
fi

if ! mkdir $MOUNTDIR ; then
    echo "failed to make directory: $MOUNTDIR" >&2
    exit 1
fi


function check_mount_with_opt()
{
    local OPT="$1"
    local MUSTBE="$2"

    echo "testing to mount spufs with ${OPT:-no} options"
    if ! mount -t spufs ${OPT:+-o $OPT} none $MOUNTDIR ; then
	echo "FAIL: failed to mount" >&2
	exit 1
    fi

    local STAT=`stat --format="%u %g %a" $MOUNTDIR`
    if [ ! $? ]; then
	echo "FAIL: failed to stat $MOUNTDIR" >&2
	exit 1
    fi
    echo "stat : '$STAT'"
    if [ "$STAT" != "$MUSTBE" ] ; then
	echo "FAIL: incorrect file stat: expected='$MUSTBE'" >&2
	umount $MOUNTDIR
	exit 1
    fi

    if ! umount $MOUNTDIR ; then
	echo "FAIL: failed to umount" >&2
	exit 1
    fi
}

NOBODY_UID=`id -u nobody`
NOBODY_GID=`id -g nobody`

echo "nobody: uid($NOBODY_UID) gid($NOBODY_GID)"

umount /spu

check_mount_with_opt "" "0 0 775"
check_mount_with_opt "mode=777" "0 0 777"
check_mount_with_opt "uid=$NOBODY_UID,mode=0700" "$NOBODY_UID 0 700"
check_mount_with_opt "gid=$NOBODY_GID,mode=70" "0 $NOBODY_GID 70"
check_mount_with_opt "uid=$NOBODY_UID,gid=$NOBODY_GID,mode=0750" "$NOBODY_UID $NOBODY_GID 750"

mount /spu

echo "done."
exit 0


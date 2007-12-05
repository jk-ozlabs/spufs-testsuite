#!/bin/bash
#
# Testsuite for the Linux SPU filesystem
#
#  Copyright (C) IBM Corporation, 2007
#
#  Author: Jeremy Kerr <jk@ozlabs.org>
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


# make sure we can umount and mount spufs

if ! umount /spu
then
	echo "Can't 'unmount spufs'" >&2
	exit 1
fi

if grep '^spufs ' /proc/mounts
then
	echo "spufs still mounted, after 'umount /spu'?'" >&2
	exit 1
fi

if ! mount /spu
then
	echo "Can't mount spufs after umount" >&2
	exit 1
fi

if ! grep '^spufs ' /proc/mounts
then
	echo "Mounted spufs, but can't find it in /proc/mounts"
	exit 1
fi

exit 0

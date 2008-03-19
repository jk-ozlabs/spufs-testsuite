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

obj += 01-spe-get 02-spe-put 03-hugepage-spe-faulted 04-hugepage-ppe-faulted \
       05-schedule-during-fault

05-schedule-during-fault: LDFLAGS += -lpthread

$(obj): pattern.o

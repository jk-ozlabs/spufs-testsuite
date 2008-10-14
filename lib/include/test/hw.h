/*
 * Testsuite for the Linux SPU filesystem
 *
 * Copyright (C) IBM Corporation, 2007
 *
 * Author: Jeremy Kerr <jk@ozlabs.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef TEST_HW_H
#define TEST_HW_H

#include <sys/stat.h>

#define LS_SIZE	0x40000
#define PSMAP_SIZE 0x20000

#define PSMAP_SIGNAL1_OFFSET 0x1400c
#define PSMAP_SIGNAL2_OFFSET 0x1c00c

static inline int count_spes(void)
{
	struct stat statbuf;
	int rc;

	rc = stat("/sys/devices/system/spu", &statbuf);
	assert(!rc);

	return statbuf.st_nlink - 2;
}

#endif /* TEST_HW_H */

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
#ifndef _TEST_UTILS_H
#define _TEST_UTILS_H

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

static int name_i = 0;

char *name_spu_context(const char *gang_name)
{
	int rc;
	char *name;

	if (gang_name == NULL)
		gang_name = "/spu";
	rc = asprintf(&name, "%s/ctx-%05d-%03d", gang_name, getpid(), name_i++);

	assert(rc != -1);
	return name;
}

char *name_spu_gang(void)
{
	int rc;
	char *name;

	rc = asprintf(&name, "/spu/gang-%05d-%03d", getpid(), name_i++);

	assert(rc != -1);
	return name;
}

#endif /* _TEST_UTILS_H */

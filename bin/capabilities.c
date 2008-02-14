/*
 * Testsuite for the Linux SPU filesystem
 *
 * Copyright (C) IBM Corporation, 2008
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


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

static int cap_always(void)
{
	return 1;
}

static int cap_never(void)
{
	return 0;
}

static int cap_root(void)
{
	return getuid() == 0;
}

static int cap_affinity(void)
{
	struct stat statbuf;

	/* test to see if we have NUMA nodes in /sys */
	return stat("/sys/devices/system/node", &statbuf) == 0;
}

struct capability {
	const char *name;
	int (*fn)(void);
	int value;
	int valid;
};

static struct capability caps[] = {
	{
		.name = "always",
		.fn = cap_always
	},
	{
		.name = "never",
		.fn = cap_never
	},
	{
		.name = "root",
		.fn = cap_root
	},
	{
		.name = "affinity",
		.fn = cap_affinity
	},
	{
		.name = NULL
	}
};

static int __cap_available(struct capability *cap)
{
	if (!cap->valid) {
		cap->value = cap->fn();
		cap->valid = 1;
	}

	return cap->value;
}

int cap_available(const char *name)
{
	struct capability *cap;

	for (cap = caps; cap->name; cap++) {
		if (!strcmp(cap->name, name))
			return __cap_available(cap);
	}

	return 0;
}

#if 0

void print_all_caps(void)
{
	struct capability *cap;

	for (cap = caps; cap->name; cap++)
		printf("%s=%d\n", cap->name, get_capability(cap));
}

int print_cap(const char *name)
{
	struct capability *cap;

	for (cap = caps; cap->name; cap++) {
		if (!strcasecmp(cap->name, name)) {
			printf("%s=%d\n", cap->name, get_capability(cap));
			return 0;
		}
	}

	fprintf(stderr, "No such capability '%s'\n", name);
	return -1;
}

#endif


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

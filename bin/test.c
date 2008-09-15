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

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>

#include "talloc/talloc.h"
#include "test.h"
#include "capabilities.h"

/* give benchmarks more time to run */
#define DEFAULT_TEST_TIMEOUT		10
#define DEFAULT_BENCHMARK_TIMEOUT	60

#define streq(a,b)	(!strcmp((a),(b)))

struct test *test_create(void *ctx, char *path, int type)
{
	struct test *test;

	test = talloc(ctx, struct test);
	test->type = type;
	test->path = path;
	talloc_steal(test, path);

	/* set defaults */
	test->expected_rc = 0;
	test->disabled = 0;
	test->caps_required = NULL;
	test->n_caps_required = 0;

	test->output.buf = NULL;
	test->output.len = 0;
	test->output.bufsize = 0;

	test->run.stdout_fd = -1;

	test->result = TEST_NOTRUN;

	if (type == TEST_TYPE_BENCHMARK)
		test->timeout = DEFAULT_BENCHMARK_TIMEOUT;
	else
		test->timeout = DEFAULT_TEST_TIMEOUT;

	return test;
}

void test_add_required_cap(struct test *test, char *cap, int invert)
{
	int n;

	n = test->n_caps_required++;
	test->caps_required = talloc_realloc(test, test->caps_required,
			struct required_cap, test->n_caps_required);
	test->caps_required[n].cap = cap;
	test->caps_required[n].invert = invert;
}

void test_header(struct test *test)
{
	printf("%-50s", test->path);
	fflush(stdout);
}

void test_pass(struct test *test)
{
	test->result = TEST_PASSED;

	if (test->type == TEST_TYPE_TEST) {
		printf("PASS\n");
	} else {
		int end;
		char *result_time = talloc_strndup(test, test->output.buf,
				test->output.len + 1);

		while (isspace(*result_time))
			result_time++;

		result_time[test->output.len] = '\0';

		end = test->output.len - 1;
		while (isspace(result_time[end])) {
			result_time[end] = '\0';
			end --;
		}

		printf("%s\n", result_time);
	}
}

void test_fail(struct test *test, char *fmt, ...)
{
	va_list ap;
	char *msg;

	va_start(ap, fmt);
	msg = talloc_vasprintf(NULL, fmt, ap);
	va_end(ap);

	test->result = TEST_FAILED;
	printf("FAIL    (%s)\n", msg);

	if (test->output.len > 0) {
		printf("test output:\n");
		fwrite(test->output.buf, sizeof(char),
				test->output.len, stdout);
	}

	talloc_free(msg);
}

void test_skip(struct test *test, char *fmt, ...)
{
	va_list ap;
	char *msg;

	va_start(ap, fmt);
	msg = talloc_vasprintf(NULL, fmt, ap);
	va_end(ap);

	test->result = TEST_SKIPPED;
	printf("SKIPPED (%s)\n", msg);

	talloc_free(msg);
}

static int is_param_str(char *str, char *param, char **value_str)
{
	int len = strlen(param);
	if (strncmp(str, param, len))
		return 0;

	if (str[len] != '=')
		return 0;

	/* store start of parameter value in value_str*/
	*value_str = str + len + 1;

	return 1;

}

static void handle_testconf_token(struct test *test, char *tok)
{
	char *val;

	/* handle flags */
	if (streq(tok, "disabled")) {
		test->disabled = 1;
		return;
	}

	/* handle parameters */
	if (is_param_str(tok, "timeout", &val)) {
		test->timeout = atoi(val);
		return;

	}

	if (is_param_str(tok, "expected_rc", &val)) {
		test->expected_rc = atoi(val);
		return;

	}

	if (is_param_str(tok, "signal", &val)) {
		test->expected_sig = atoi(val);
		return;

	}

	/* handle capabilities */
	if (!strncmp(tok, "cap_", strlen("cap_"))) {
		test_add_required_cap(test, tok + strlen("cap_"), 0);
		return;
	}

	if (!strncmp(tok, "!cap_", strlen("!cap_"))) {
		test_add_required_cap(test, tok + strlen("!cap_"), 1);
		return;
	}

	fprintf(stderr, "Unknown test parameter %s\n", tok);
}

void test_read_config(struct test *test)
{
	char *c, *dir, *file, *testconffile, *filepattern;
	int fd, len, fp_len;
	char buf[4096];

	dir = talloc_strdup(NULL, test->path);
	c = strrchr(dir, '/');
	if (!c)
		return;

	*c = '\0';
	file = c + 1;

	testconffile = talloc_asprintf(dir, "%s/tests.conf", dir);

	fd = open(testconffile, O_RDONLY);
	if (fd < 0) {
		talloc_free(dir);
		return;
	}

	/* fixme: config files larger that 4096 bytes */
	len = read(fd, buf, sizeof(buf));

	close(fd);

	c = buf;
	filepattern = talloc_asprintf(dir, "%s:", file);
	fp_len = strlen(filepattern);

	for (c = buf; c && *c;) {
		char *newline;
		int match = 0;

		/* skip whitespace */
		while (isspace(*c))
			c++;

		newline = strchr(c, '\n');

		/* if it's a comment, skip this line */
		if (*c == '#') {

		/* we only want lines begining with "file:" ... */
		} else if (!strncmp(c, filepattern, fp_len)) {
			c += fp_len;
			match = 1;

		/* ... or *:, which matches all tests in this dir: */
		} else if (!strncmp(c, "*:", 2)) {
			c += 2;
			match = 1;

		}

		if (match) {
			char *tok;
			char *delim = " \t";

			/* null-terminate this line */
			if (newline)
				*newline = '\0';

			for (tok = strtok(c, delim); tok;
					tok = strtok(NULL, delim))
				handle_testconf_token(test, tok);
		}

		c = newline;
		if (c)
			c++;
	}
}

int test_should_run(struct test *test)
{
	int x;

	if (test->disabled) {
		test_skip(test, "disabled");
		return 0;
	}

	for (x = 0; x < test->n_caps_required; x++) {
		struct required_cap *req_cap = &test->caps_required[x];

		if (!!cap_available(req_cap->cap) == !!req_cap->invert) {
			test_skip(test, "requires %s%s",
					req_cap->invert ? "!" : "",
					req_cap->cap);
			test->result = TEST_SKIPPED;
			return 0;
		}
	}

	return 1;
}



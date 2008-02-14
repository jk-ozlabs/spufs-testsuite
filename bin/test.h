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

#ifndef _TEST_H
#define _TEST_H

#include "talloc/talloc.h"

struct test {
	enum {
		TEST_TYPE_TEST,
		TEST_TYPE_BENCHMARK
	} type;
	char *path;
	int expected_rc;
	int expected_sig;
	int timeout;
	int disabled;

	struct required_cap {
		char *cap;
		int invert;
	} *caps_required;
	int n_caps_required;

	struct {
		int stdout_fd;
		int pid;
		char *path;
	} run;

	struct {
		char *buf;
		int len;
		int bufsize;
	} output;

	enum {
		TEST_NOTRUN,
		TEST_PASSED,
		TEST_FAILED,
		TEST_SKIPPED
	} result;
};

struct test *test_create(void *ctx, char *path, int type);

void test_add_required_cap(struct test *test, char *cap, int invert);

void test_header(struct test *test);

void test_pass(struct test *test);

void test_fail(struct test *test, char *fmt, ...);

void test_skip(struct test *test, char *fmt, ...);

void test_read_config(struct test *test);

int test_should_run(struct test *test);

#endif /* _TEST_H */

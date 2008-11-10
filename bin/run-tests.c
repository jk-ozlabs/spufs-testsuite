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

#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/klog.h>

#include "talloc/talloc.h"
#include "capabilities.h"
#include "test.h"


#define TEST_DIR	"tests"
#define BENCHMARK_DIR	"benchmarks"

#define max(x, y) ((x) > (y) ? (x) : (y))

static int qsort_testcmp(const void *a, const void *b)
{
	return strcmp((*(const struct test **)a)->path,
			(*(const struct test **)b)->path);
}

static int is_test(char *file, struct stat *statbuf)
{
	return S_ISREG(statbuf->st_mode) &&
			(statbuf->st_mode & S_IXUSR) &&
			isdigit(file[0]) && isdigit(file[1]);
}

static struct test **find_tests(char *testpath, int type)
{
	int n_tests, n_dirs, d;
	struct dirent *dirent;
	struct test **tests;
	struct stat statbuf;
	char **dirs;
	DIR *dir;

	tests = NULL;
	n_tests = 0;

	dirs = talloc_realloc(NULL, NULL, char *, 1);
	dirs[0] = testpath;
	n_dirs = 1;

	for (d = 0; d < n_dirs; d++) {

		dir = opendir(dirs[d]);
		if (!dir) {
			perror("opendir");
			return NULL;
		}

		while ((dirent = readdir(dir))) {
			char *path, *file;

			file = dirent->d_name;

			if (*file == '.')
				continue;

			path = talloc_asprintf(NULL, "%s/%s", dirs[d], file);

			if (stat(path, &statbuf)) {
				talloc_free(path);
				continue;
			}

			if (S_ISDIR(statbuf.st_mode)) {
				dirs = talloc_realloc(NULL, dirs,
						char *, n_dirs + 1);
				talloc_steal(dirs, path);
				dirs[n_dirs] = path;
				n_dirs++;

			} else	if (is_test(file, &statbuf)) {

				tests = talloc_realloc(NULL, tests,
						struct test *, n_tests + 1);
				tests[n_tests] = test_create(tests, path, type);
				n_tests++;

			} else {
				talloc_free(path);
			}

		}
		closedir(dir);
	}

	qsort(tests, n_tests, sizeof(struct test *), qsort_testcmp);

	tests = talloc_realloc(NULL, tests, struct test *, n_tests + 1);
	tests[n_tests] = NULL;

	talloc_free(dirs);

	return tests;
}

void sigchld_handler(int signum)
{
	/* we don't need to do anything here, just break out of the sleep() */
}

static int read_test_output(struct test *test, int timeout)
{
	const int readsize = 4096;
	struct pollfd fds[1];
	int rc, n_fds = 0;

	if (test->run.stdout_fd > 0) {
		fds[0].fd = test->run.stdout_fd;
		fds[0].events = POLLIN | POLLERR | POLLHUP;
		n_fds++;
	}

	rc = poll(fds, n_fds, timeout);

	if (rc < 0) {
		if (errno != EINTR)
			perror("poll");
		return 0;
	}

	if (rc == 0)
		return 0;

	if (fds[0].revents & POLLERR)
		return -1;

	if (test->output.bufsize - test->output.len < readsize) {
		int alloc_size = max(readsize, test->output.bufsize * 2);

		test->output.buf = talloc_realloc(test, test->output.buf,
				char, alloc_size);

		test->output.bufsize = alloc_size;
	}

	rc = read(test->run.stdout_fd,
			test->output.buf + test->output.len, readsize);

	if (rc < 0) {
		perror("read");
		return -1;
	} else if (rc == 0) {
		return -1;
	} else {
		test->output.len += rc;
	}

	return 0;
}

static int dmesg_check()
{
	static char *buf;
	static int bufsize;
	int kernel_bufsize, rc;

	kernel_bufsize = klogctl(10, NULL, 0);
	if (kernel_bufsize != bufsize) {
		bufsize = kernel_bufsize;
		buf = talloc_realloc(NULL, buf, char, bufsize);
	}

	rc = klogctl(3, buf, bufsize);
	if (rc < 0) {
		perror("klogctl");
		return -1;
	}

	return (strstr(buf, "Badness") != NULL);
}

static int run_test(struct test *test, int dir_fd, int enable_timeout)
{
	int rc, pipe_fds[2];
	struct timeval tv, timeout_tv;
	siginfo_t siginfo;
	struct sigaction sa = {
		.sa_handler = sigchld_handler,
	};

	test_header(test);

	test_read_config(test);
	if (!test_should_run(test)) {
		return 0;
	}

	if (pipe(pipe_fds)) {
		perror("pipe");
		return -1;
	}

	test->run.stdout_fd = pipe_fds[0];

	test->run.pid = fork();
	if (test->run.pid < 0) {
		perror("fork");
		return -1;
	}

	if (test->run.pid == 0) {
		/* child process: exec the test */
		char *arg, *path, *dir, *c;

		close(pipe_fds[0]);

		close(STDIN_FILENO);
		if (dup2(pipe_fds[1], STDOUT_FILENO) < 0) {
			perror("dup 2");
			exit(EXIT_FAILURE);
		}
		if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) {
			perror("dup 1");
			exit(EXIT_FAILURE);
		}

		close(pipe_fds[1]);

		if (fchdir(dir_fd)) {
			perror("fchdir");
			exit(EXIT_FAILURE);
		}

		dir = talloc_strdup(NULL, test->path);
		c = strrchr(dir, '/');
		if (c) {
			*c = '\0';
			path = talloc_asprintf(dir, "../%s:%s",
					dir, getenv("PATH"));
			setenv("PATH", path, 1);
		}
		talloc_free(dir);

		arg = talloc_asprintf(test->path, "../%s", test->path);
		execl(arg, arg, NULL);
		exit(EXIT_FAILURE);
	}

	close(pipe_fds[1]);

	if (sigaction(SIGCHLD, &sa, NULL)) {
		perror("sigaction");
		return -1;
	}

	gettimeofday(&timeout_tv, NULL);

	timeout_tv.tv_sec += test->timeout;

	for (;;) {
		int timeout;

		siginfo.si_pid = 0;

		rc = waitid(P_PID, test->run.pid, &siginfo, WEXITED | WNOHANG);

		if (rc != 0) {
			perror("waitpid");
			break;
		}

		/* if the test has exited, check the exit status */
		if (siginfo.si_pid) {
			/* exited due to exit() */
			if (siginfo.si_code == CLD_EXITED) {
				if (siginfo.si_status != test->expected_rc) {
					test_fail(test, "rc %d, "
							"expected %d",
							siginfo.si_status,
							test->expected_rc);
					rc = -1;

				} else if (test->expected_sig) {
					test_fail(test, "expected sig %d",
							test->expected_sig);
					rc = -1;

				} else if (dmesg_check()) {
					test_fail(test, "dmesg");
					rc = -1;

				} else {
					test_pass(test);
					rc = 0;
				}

			/* exited due to signal */
			} else if ((siginfo.si_code == CLD_KILLED)) {
				if (siginfo.si_status == test->expected_sig) {
					test_pass(test);
					rc = 0;
				} else if (siginfo.si_status == SIGALRM) {
					test_fail(test, "timeout");
					rc = -1;
				} else {
					test_fail(test, "killed, sig %d",
							siginfo.si_status);
					rc = -1;
				}
			}
			test->run.pid = 0;
			break;
		}

		/* see if we've timed-out */
		gettimeofday(&tv, NULL);
		if (enable_timeout && timercmp(&tv, &timeout_tv, >)) {
			kill(test->run.pid, SIGALRM);
		}

		timeout = 1 + (1000 * (timeout_tv.tv_sec - tv.tv_sec));

		read_test_output(test, timeout);
	}

	close(test->run.stdout_fd);

	return rc;
}

static int run_tests(struct test **tests, int cont_on_failure,
		int enable_timeout)
{
	struct test **test;
	char working_dir[256];
	int dirfd, rc;
	time_t t;
	struct tm *tm;

	if (dmesg_check()) {
		fprintf(stderr, "dmesg check failed: not running tests\n");
		return -1;
	}

	t = time(NULL);
	tm = localtime(&t);

	strftime(working_dir, sizeof(working_dir) - 1,
			"run.%Y-%m-%d.%H:%m:%S", tm);

	if (mkdir(working_dir, 0755) && errno != EEXIST) {
		perror("mkdir");
		return -1;
	}

	dirfd = open(working_dir, O_RDONLY);
	if (dirfd < 0) {
		perror("open");
		return -1;
	}

	rc = 0;

	for (test = tests; *test; test++) {

		rc |= run_test(*test, dirfd, enable_timeout);

		if (rc && !cont_on_failure)
			return -1;
	}

	close(dirfd);

	return rc;
}

static void usage(const char *progname)
{
	fprintf(stderr, "usage: %s [--benchmark] [--continue] [--no-timeout] [testdir]\n",
			progname);
}


static void print_summary(struct test **tests)
{
	int n_tests, n_pass, n_fail, n_skip;
	struct test **test;

	n_tests = n_pass = n_fail = n_skip = 0;

	for (test = tests; *test; test++) {
		if ((*test)->result == TEST_NOTRUN)
			continue;

		if ((*test)->ignored)
			continue;

		n_tests++;
		if ((*test)->result == TEST_PASSED)
			n_pass++;
		else if ((*test)->result == TEST_FAILED)
			n_fail++;
		else if ((*test)->result == TEST_SKIPPED)
			n_skip++;

	}

	printf("%d tests run: %d passed, %d failed, %d skipped\n",
			n_tests, n_pass, n_fail, n_skip);
}

struct option options[] = {
	{
		.name = "continue",
		.has_arg = 0,
		.val = 'c'
	},
	{
		.name = "benchmark",
		.has_arg = 0,
		.val = 'b'
	},
	{
		.name = "capabilities",
		.has_arg = 0,
		.val = 'p'
	},
	{
		.name = "help",
		.has_arg = 0,
		.val = 'h'
	},
	{
		.name = "no-timeout",
		.has_arg = 0,
		.val = 't'
	},
	{
		.name = NULL,
		.val = 0
	}
};


int main(int argc, char **argv)
{
	int benchmark, cont, enable_timeout;
	struct test **tests, **benchmarks;
	char *testdir;
	int c, rc;

	cont = 0;
	benchmark = 0;
	enable_timeout = 1;

	for (;;) {
		c = getopt_long(argc, argv, "cbtph", options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'c':
			cont = 1;
			break;
		case 'b':
			benchmark = 1;
			break;
		case 't':
			enable_timeout = 0;
			break;
		case 'p':
			print_capabilities(stdout);
			return EXIT_SUCCESS;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			usage(argv[0]);
			return EXIT_FAILURE;
		}
	}

	testdir = TEST_DIR;

	if (optind == argc - 1) {
		testdir = argv[optind];

	} else if (optind != argc) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	tests = find_tests(testdir, TEST_TYPE_TEST);
	if (!tests) {
		fprintf(stderr, "No tests found\n");
		return EXIT_FAILURE;
	}

	rc = run_tests(tests, cont, enable_timeout);

	print_summary(tests);
	talloc_free(tests);

	if (rc || !benchmark)
		return rc;

	printf("--- Running benchmarks\n");
	benchmarks = find_tests(BENCHMARK_DIR, TEST_TYPE_BENCHMARK);
	run_tests(benchmarks, 0, 0);
	talloc_free(benchmarks);

	return 0;
}

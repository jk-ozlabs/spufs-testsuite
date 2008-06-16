
#define FUSE_USE_VERSION 26

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <pthread.h>

#include <talloc/talloc.h>

#include <fuse.h>
#include <fuse_lowlevel.h>


#define FILE_SIZE 4096
static const char *file_name = "/slowfs";

struct slowfs {
	struct fuse *fuse;
	struct fuse_chan *chan;
	char *mountpoint;
	int fuse_rc;
	pthread_t pthread;
};

static int slowfs_getattr(const char *path, struct stat *statbuf)
{
	int rc = -ENOENT;

	memset(statbuf, 0, sizeof(*statbuf));

	if (!strcmp(path, "/")) {
		statbuf->st_mode = S_IFDIR | 0755;
		statbuf->st_nlink = 2;
		rc = 0;

	} else if (!strcmp(path, file_name)) {
		statbuf->st_mode = S_IFREG | 0444;
		statbuf->st_nlink = 1;
		statbuf->st_size = FILE_SIZE;
		rc = 0;
	}

	return rc;

}

static int slowfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset __attribute__((unused)),
		struct fuse_file_info *fi __attribute__((unused)))
{

	if (strcmp(path, "/"))
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, file_name + 1, NULL, 0);

	return 0;
}

static int slowfs_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path, file_name) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}


static int slowfs_read(const char *path, char *buf, size_t len, off_t offset,
		struct fuse_file_info *fi __attribute__((unused)))
{
	sleep(2);
	memset(buf, 0, len);
	return len;
}

static struct fuse_operations ops = {
	.getattr	= slowfs_getattr,
	.readdir	= slowfs_readdir,
	.open		= slowfs_open,
	.read		= slowfs_read,
};

static void *slowfs_fn(void *arg)
{
	struct slowfs *slowfs = arg;

	slowfs->fuse_rc = fuse_loop(slowfs->fuse);

	return slowfs;
}

struct slowfs *slowfs_mount(const char *mountpoint)
{
	struct slowfs *slowfs;

	slowfs = talloc(NULL, struct slowfs);
	if (!slowfs) {
		perror("talloc");
		goto out_free;
	}

	slowfs->mountpoint = talloc_strdup(slowfs, mountpoint);
	if (!slowfs->mountpoint) {
		perror("talloc");
		goto out_free;
	}

	slowfs->chan = fuse_mount(mountpoint, NULL);
	if (!slowfs->chan) {
		perror("fuse_mount");
		goto out_free;
	}

	slowfs->fuse = fuse_new(slowfs->chan, NULL, &ops, sizeof(ops), NULL);
	if (!slowfs->fuse) {
		perror("fuse_new");
		goto out_free;
	}

	if (fuse_daemonize(1)) {
		perror("fuse_daemonize");
		goto out_free;
	}

	if (pthread_create(&slowfs->pthread, NULL, slowfs_fn, slowfs)) {
		perror("pthread_create");
		goto out_free;
	}

	/* fuse_set_signal_handlers() ? */

	return slowfs;

out_free:
	talloc_free(slowfs);
	return NULL;
}

void slowfs_unmount(struct slowfs *slowfs)
{
	fuse_exit(slowfs->fuse);
	fuse_unmount(slowfs->mountpoint, slowfs->chan);
	pthread_join(slowfs->pthread, NULL);
	talloc_free(slowfs);
}



#define FUSE_USE_VERSION 26

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fuse.h>

#define FILE_SIZE 4096
static const char *file_name = "/slowfs";

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

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &ops, NULL);
}

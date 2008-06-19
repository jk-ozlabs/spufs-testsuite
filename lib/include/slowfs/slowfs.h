#ifndef _SLOWFS_H
#define _SLOWFS_H

struct slowfs;

struct slowfs *slowfs_mount(const char *mountpoint);

void slowfs_unmount(struct slowfs *slowfs);

#endif /* _SLOWFS_H */

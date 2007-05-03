#ifndef _SPU_SYSCALLS_H
#define _SPU_SYSCALLS_H

#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#define SPU_CREATE_EVENTS_ENABLED	0x0001
#define SPU_CREATE_GANG			0x0002
#define SPU_CREATE_NOSCHED		0x0004
#define SPU_CREATE_ISOLATE		0x0008
#define SPU_CREATE_AFFINITY_SPU		0x0010
#define SPU_CREATE_AFFINITY_MEM		0x0020

/* while the spu syscalls aren't widely available... */

static inline int spu_create(const char *name, unsigned long flags,
		int mode, ...)
{
	va_list ap;
	int fd = 0;

	if (flags & SPU_CREATE_AFFINITY_SPU) {
		va_start(ap, mode);
		fd = va_arg(ap, int);
		va_end(ap);
		printf("%s: gang fd: %d\n", __FUNCTION__, fd);
		return syscall(SYS_spu_create, name, flags, mode, fd);
	}

	return syscall(SYS_spu_create, name, flags, mode);

}

static inline int spu_run(int fd, uint32_t *npc, uint32_t *status)
{
	return syscall(SYS_spu_run, fd, npc, status);
}

#endif /* _SPU_SYSCALLS_H */

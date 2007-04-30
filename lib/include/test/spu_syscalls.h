#ifndef _SPU_SYSCALLS_H
#define _SPU_SYSCALLS_H

#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>

/* while the spu syscalls aren't widely available... */

static inline int spu_create(const char *name, unsigned long flags, int mode)
{
	return syscall(SYS_spu_create, name, flags, mode);

}

static inline int spu_run(int fd, uint32_t *npc, uint32_t *status)
{
	return syscall(SYS_spu_run, fd, npc, status);
}

#endif /* _SPU_SYSCALLS_H */

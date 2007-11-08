/*
 * Testsuite for the Linux SPU filesystem
 *
 * Copyright (C) IBM Corporation, 2007
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

#define SPE_EVENT_DMA_ALIGNMENT		0x0008
#define SPE_EVENT_SPE_ERROR		0x0010
#define SPE_EVENT_SPE_DATA_SEGMENT	0x0020
#define SPE_EVENT_SPE_DATA_STORAGE	0x0040
#define SPE_EVENT_INVALID_DMA		0x0800

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
		return syscall(SYS_spu_create, name, flags, mode, fd);
	}

	return syscall(SYS_spu_create, name, flags, mode);

}

static inline int spu_run(int fd, uint32_t *npc, uint32_t *status)
{
	return syscall(SYS_spu_run, fd, npc, status);
}

#endif /* _SPU_SYSCALLS_H */

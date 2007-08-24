#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <elf.h>

#if 0
#define prdebug(fmt...) printf(fmt)
#else
#define prdebug(fmt...) do { } while (0);
#endif


/*
 * { "lslr", NULL, __spufs_lslr_get, 19 },
 * { "decr", NULL, __spufs_decr_get, 19 },
 * { "decr_status", NULL, __spufs_decr_status_get, 19 },
 * { "signal1_type", NULL, __spufs_signal1_type_get, 19 },
 * { "signal2_type", NULL, __spufs_signal2_type_get, 19 },
 * { "event_mask", NULL, __spufs_event_mask_get, 19 },
 * { "event_status", NULL, __spufs_event_status_get, 19 },
 * { "object-id", NULL, __spufs_object_id_get, 19 },
 */

static int check_spu_note(char *p, Elf32_Nhdr *nhdr)
{
	unsigned long long tmp;
	char *s, *end;

	s = strrchr(p, '/');
	if (!s) {
		fprintf(stderr, "Error: couldn't find '/' in note name.\n");
		return 1;
	}

	s++;

	if (strcmp("lslr", s)
	    && strcmp("decr", s)
	    && strcmp("decr_status", s)
	    && strcmp("signal1_type", s)
	    && strcmp("signal2_type", s)
	    && strcmp("event_mask", s)
	    && strcmp("event_status", s)
	    && strcmp("object-id", s))
	{
		/* It's a binary-format note */
		return 0;
	}

	if (nhdr->n_descsz != 19) {
		fprintf(stderr, "Error: note %s is not 19 bytes in size.\n", p);
		return 1;
	}

	s = p + roundup(nhdr->n_namesz, 4);
	tmp = strtoull(s, &end, 16);

	if (end == s) {
		fprintf(stderr, "Error: no bytes converted for %s.\n", p);
		return 1;
	} else if (*end != '\0') {
		fprintf(stderr, "Error: not all bytes converted for %s.\n", p);
		return 1;
	}

	prdebug(" value 0x%llx", tmp);

	return 0;
}

static int find_notes(char *buf)
{
	char *p, *nstart;
	Elf32_Phdr *phdr;
	Elf32_Nhdr *nhdr;
	int status = 0;

	phdr = (Elf32_Phdr*)(buf + sizeof(Elf32_Ehdr));

	prdebug("notes at %x size %x\n", phdr->p_offset, phdr->p_filesz);

	p = nstart = buf + phdr->p_offset;

	while (p - nstart < phdr->p_filesz) {
		nhdr = (Elf32_Nhdr*)p;

		prdebug("note at 0x%x, descsz 0x%x",
			p - nstart + phdr->p_offset, nhdr->n_descsz);
		p += roundup(sizeof(*nhdr), 4);
		prdebug(" name \"%s\"", p);

		if (strncmp("SPU/", p, 4) == 0)
			status |= check_spu_note(p, nhdr);

		prdebug("\n");

		p += roundup(nhdr->n_namesz, 4);
		p += roundup(nhdr->n_descsz, 4);
	}

	return status;
}

int main(int argc, char *argv[])
{
	struct stat sbuf;
	int fd, rc;
	char *buf;

	if (argc != 2) {
		fprintf(stderr, "Usage: readnotes <core file>\n");
		return 1;
	}

	fd = open(argv[1], O_RDONLY);
	if (!fd) {
		perror("open");
		return 2;
	}

	if (fstat(fd, &sbuf)) {
		perror("fstat");
		return 3;
	}

	buf = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (!buf) {
		perror("mmap");
		return 4;
	}

	rc = find_notes(buf);

	munmap(buf, sbuf.st_size);

	return rc;
}

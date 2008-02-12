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

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#include <getopt.h>
#include <elf.h>

#define SPU_NOTE_PREFIX "SPU/"

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

/* notes are the same struct on both 32- and 64- bit. if not, we hit the
 * BUILD_BUG_ON in parse_notes() */
typedef Elf32_Nhdr Elf_Nhdr;

struct options {
	enum {
		LIST_NOTES,
		CHECK_NOTES,
		DUMP_NOTE,
		FIND_EXEC,
	} action;
	const char *dump_note_name;
};

struct spu_note_check {
	const char	*name;
	int		size;
	int		(*fn)(Elf_Nhdr *nhdr);
};

/* note sizes are padded to 4-byte alignment */
static int pad(int x)
{
	return (x + 3) & ~3;
}

static const char *note_name(Elf_Nhdr *nhdr)
{
	return (void *)nhdr + sizeof(*nhdr);
}

static const char *note_desc(Elf_Nhdr *nhdr)
{
	return (void *)nhdr + sizeof(*nhdr) + pad(nhdr->n_namesz);
}

int check_hex_note(Elf_Nhdr *nhdr)
{
	const char *name, *desc;
	int i, len;

	/* 64-bit number, plus NULL */
	len = strlen("0x0123456789abcdef") + 1;
	name = note_name(nhdr);
	desc = note_desc(nhdr);

	if (nhdr->n_descsz != len) {
		fprintf(stderr, "wrong size (%d) for note %s\n",
				nhdr->n_descsz, name);
		return -1;
	}

	if (desc[0] != '0' || desc[1] != 'x') {
		fprintf(stderr, "Invalid format for note %s: "
				"no leading '0x'\n",
				name);
		return -1;
	}

	for (i = 2; i < len - 1; i++) {
		if (!isxdigit(desc[i])) {
			fprintf(stderr, "Invalid format for note %s: "
					"invalid char '%c'\n",
					name, desc[i]);
			return -1;
		}
	}

	return 0;
}

struct spu_note_check note_checks[] = {
	{ .name = "regs",		.size = 0x800,		},
	{ .name = "fpcr",		.size = 0x10,		},
	{ .name = "lslr",		.fn   = check_hex_note	},
	{ .name = "decr",		.fn   = check_hex_note	},
	{ .name = "decr_status",	.fn   = check_hex_note	},
	{ .name = "mem",		.size = 0x40000		},
	{ .name = "signal1",		.size = 0x4		},
	{ .name = "signal1_type",	.fn   = check_hex_note	},
	{ .name = "signal2",		.size = 0x4		},
	{ .name = "signal2_type",	.fn   = check_hex_note	},
	{ .name = "event_mask",		.fn   = check_hex_note	},
	{ .name = "event_status",	.fn   = check_hex_note	},
	{ .name = "mbox_info",		.size = 0x4		},
	{ .name = "ibox_info",		.size = 0x4		},
	{ .name = "wbox_info",		.size = 0x10		},
	{ .name = "dma_info",		.size = 0x228		},
	{ .name = "proxydma_info",	.size = 0x118		},
	{ .name = "object-id",		.fn   = check_hex_note	},
	{ .name = "npc",		.fn   = check_hex_note	},
	{ .name = NULL },
};

static int check_note(Elf_Nhdr *nhdr)
{
	const char *name, *spu_name, *file_name, *c;
	struct spu_note_check *check;

	name = note_name(nhdr);
	spu_name = name + strlen(SPU_NOTE_PREFIX);
	file_name = strrchr(name, '/');
	if (!file_name) {
		fprintf(stderr, "Invalid note name %s\n", name);
		return -1;
	}

	for (c = spu_name; c < file_name; c++) {
		if (!isdigit(*c)) {
			fprintf(stderr, "Invalid note name %s\n", name);
			return -1;
		}
	}

	file_name++;

	for (check = note_checks; check->name != NULL; check++) {
		if (strcmp(file_name, check->name))
			continue;

		if (check->size && check->size != nhdr->n_descsz) {
			fprintf(stderr, "Invalid size (%d) for note %s\n",
					nhdr->n_descsz, name);
			return -1;
		}

		if (check->fn)
			return check->fn(nhdr);

		return 0;
	}

	/* allow unknown notes */
	return 0;
}

static int parse_notes(void *notes, unsigned int notes_size,
		struct options *opts)
{
	Elf_Nhdr *nhdr;
	int offset = 0;

	BUILD_BUG_ON(sizeof(Elf_Nhdr) != sizeof(Elf32_Nhdr));
	BUILD_BUG_ON(sizeof(Elf_Nhdr) != sizeof(Elf64_Nhdr));

	for (nhdr = notes; offset < notes_size; nhdr = notes + offset) {
		int namesz = pad(nhdr->n_namesz);
		int descsz = pad(nhdr->n_descsz);
		char *name;

		if (offset + sizeof(*nhdr) + namesz > notes_size) {
			fprintf(stderr, "note name extends past "
					"notes section\n");
			return -1;
		}

		if (offset + sizeof(*nhdr) + namesz + descsz > notes_size) {
			fprintf(stderr, "note descriptor extends past "
					"notes section\n");
			return -1;
		}

		offset += sizeof(*nhdr) + namesz + descsz;

		if (nhdr->n_type != 0x1)
			continue;

		name = (char *)(nhdr + 1);
		if (nhdr->n_namesz < strlen(SPU_NOTE_PREFIX) + 1)
			continue;

		if (strncmp(name, SPU_NOTE_PREFIX, strlen(SPU_NOTE_PREFIX)))
			continue;

		if (opts->action == LIST_NOTES) {
			printf("0x%08x %s\n", nhdr->n_descsz,
					name + strlen(SPU_NOTE_PREFIX));
		} else if (opts->action == CHECK_NOTES) {
			if (check_note(nhdr))
				return -1;
		} else if (opts->action == DUMP_NOTE) {
			if (!strcmp(name, opts->dump_note_name) ||
					!strcmp(name + strlen(SPU_NOTE_PREFIX),
						opts->dump_note_name)) {
				write(STDOUT_FILENO, note_desc(nhdr),
						nhdr->n_descsz);
				return 0;

			}
		}
	}

	if (opts->action == DUMP_NOTE) {
		fprintf(stderr, "note %s not found\n", opts->dump_note_name);
		return -1;
	}

	return 0;
}

/**
 * return values:
 *  0:  found the only relevant section, and it passes (ie, the checking
 *      is complete)
 *  1:  this section is OK (or unknown), but we need to check more
 * -1:  this section is broken
 */
static int parse_section(void *section, int size, int type,
		struct options *opts)
{
	if (type == PT_LOAD && opts->action == FIND_EXEC) {
		if (!memcmp(section, ELFMAG, SELFMAG))
			return 0;

		/* we expect to find the ELF header at the first LOAD segment */
		return -1;

	} else if (type == PT_NOTE) {
		if (parse_notes(section, size, opts))
			return -1;
	}

	return 1;
}

static int parse_elf_32(void *file, unsigned int map_size, struct options *opts)
{
	Elf32_Ehdr *ehdr = file;
	Elf32_Phdr *phdr;
	int i, rc = 1;

	/* more sanity checks, specific to 32-bit */
	if (ehdr->e_machine != EM_PPC) {
		fprintf(stderr, "Invlid ELF file: e_machine (%d) != EM_PPC\n",
				ehdr->e_machine);
		return -1;
	}

	if (ehdr->e_phoff >= map_size) {
		fprintf(stderr, "Program header table past end of file\n");
		return -1;
	}

	if (ehdr->e_phentsize != sizeof(*phdr)) {
		fprintf(stderr, "Program header entries are %d bytes, "
				"expected %zd\n", ehdr->e_phentsize,
				sizeof(*phdr));
		return -1;
	}

	if (ehdr->e_phoff + ehdr->e_phentsize * ehdr->e_phnum > map_size) {
		fprintf(stderr, "Program header table extends past end of "
				"file\n");
		return -1;
	}

	/* now we know that the program headers are at a sensible location,
	 * we can parse them. */
	for (i = 0; i < ehdr->e_phnum; i++) {
		phdr = file + ehdr->e_phoff + i * sizeof(*phdr);

		if (phdr->p_offset > map_size) {
			fprintf(stderr, "Program header entry %d starts at "
					"0x%016" PRIx32 ", which is past "
					"end-of-file\n", i, phdr->p_offset);
			return -1;
		}

		if (phdr->p_offset + phdr->p_filesz > map_size) {
			fprintf(stderr, "Program header entry %d ends at "
					"0x%016" PRIx32 ", which is past "
					"end-of-file\n", i, phdr->p_offset);
			return -1;
		}

		rc = parse_section(file + phdr->p_offset, phdr->p_filesz,
				phdr->p_type, opts);

		if (rc < 0)
			return -1;
		if (rc == 0)
			return 0;
	}

	return 0;
}

static int parse_elf_64(void *file, unsigned int map_size, struct options *opts)
{
	Elf64_Ehdr *ehdr = file;
	Elf64_Phdr *phdr;
	int i, rc = 1;

	/* more sanity checks, specific to 64-bit */
	if (ehdr->e_machine != EM_PPC64) {
		fprintf(stderr, "Invlid ELF file: e_machine (%d) != EM_PPC64\n",
				ehdr->e_machine);
		return -1;
	}

	if (ehdr->e_phoff >= map_size) {
		fprintf(stderr, "Program header table past end of file\n");
		return -1;
	}

	if (ehdr->e_phentsize != sizeof(*phdr)) {
		fprintf(stderr, "Program header entries are %d bytes, "
				"expected %zd\n", ehdr->e_phentsize,
				sizeof(*phdr));
		return -1;
	}

	if (ehdr->e_phoff + ehdr->e_phentsize * ehdr->e_phnum > map_size) {
		fprintf(stderr, "Program header table extends past end of "
				"file\n");
		return -1;
	}

	/* now we know that the program headers lie entirely within the file,
	 * we can parse them. */
	for (i = 0; i < ehdr->e_phnum; i++) {
		phdr = file + ehdr->e_phoff + i * sizeof(*phdr);

		if (phdr->p_offset > map_size) {
			fprintf(stderr, "Program header entry %d starts at "
					"0x%016" PRIx64 ", which is past "
					"end-of-file\n", i, phdr->p_offset);
			return -1;
		}

		if (phdr->p_offset + phdr->p_filesz > map_size) {
			fprintf(stderr, "Program header entry %d ends at "
					"0x%016" PRIx64 ", which is past "
					"end-of-file\n", i, phdr->p_offset);
			return -1;
		}

		rc = parse_section(file + phdr->p_offset, phdr->p_filesz,
				phdr->p_type, opts);

		if (rc < 0)
			return -1;
		if (rc == 0)
			return 0;
	}

	return 0;
}

static int parse_elf(void *file, unsigned int map_size, struct options *opts)
{
	char *ident = file;

	/* some basic ELF sanity checks */
	if (memcmp(ident, ELFMAG, SELFMAG)) {
		fprintf(stderr, "Invalid ELF file: bad e_ident magic\n");
		return -1;
	}

	if (ident[EI_DATA] != ELFDATA2MSB) {
		fprintf(stderr, "Invalid ELF file: bad data encoding\n");
		return -1;
	}

	if (ident[EI_VERSION] != EV_CURRENT) {
		fprintf(stderr, "Invalid ELF file: bad file version (%d)\n",
				ident[EI_VERSION]);
		return -1;
	}


	if (ident[EI_CLASS] == ELFCLASS32)
		return parse_elf_32(file, map_size, opts);

	else if (ident[EI_CLASS] == ELFCLASS64)
		return parse_elf_64(file, map_size, opts);

	fprintf(stderr, "Invalid ELF file: bad elf class (0x%x)\n",
			ident[EI_CLASS]);

	return -1;
}

static void *map_file(const char *filename, unsigned int *size)
{
	struct stat statbuf;
	char *map;
	int fd;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return NULL;
	}

	if (fstat(fd, &statbuf)) {
		perror("fstat");
		close(fd);
		return NULL;
	}

	map = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (map == MAP_FAILED) {
		perror("mmap");
		return NULL;
	}

	*size = statbuf.st_size;
	return map;
}

static void usage(const char *prog)
{
	fprintf(stderr, "usage:\t%s [--list-notes] <corefile>\n", prog);
	fprintf(stderr, "\t%s --check-notes <corefile>\n", prog);
	fprintf(stderr, "\t%s --print-note <notename> <corefile>\n", prog);
	fprintf(stderr, "\t%s --find-exec <corefile>\n", prog);
}

struct option options[] = {
	{
		.name		= "list-notes",
		.has_arg	= 0,
		.val		= 'l',
	},
	{
		.name		= "check-notes",
		.has_arg	= 0,
		.val		= 'c',
	},
	{
		.name		= "print-note",
		.has_arg	= 1,
		.val		= 'p',
	},
	{
		.name		= "find-exec",
		.has_arg	= 0,
		.val		= 'f',
	},
	{
		.name		= NULL,
	}
};

int main(int argc, char **argv)
{
	unsigned int size;
	struct options opts;
	void *map;
	int opt;

	opts.action = LIST_NOTES;

	while ((opt = getopt_long(argc, argv, "lcp:f", options, NULL)) != -1) {
		switch (opt) {
		case 'l':
			opts.action = LIST_NOTES;
			break;
		case 'c':
			opts.action = CHECK_NOTES;
			break;
		case 'p':
			opts.action = DUMP_NOTE;
			opts.dump_note_name = optarg;
			break;
		case 'f':
			opts.action = FIND_EXEC;
			break;
		default:
			usage(argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (optind != argc - 1) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	map = map_file(argv[optind], &size);
	if (!map)
		return EXIT_FAILURE;

	return parse_elf(map, size, &opts);
}

/* Check LICENSE file for copyright and license details. */

/*
 * MIPS32 simulator, it receives a little-endian MIPS32 binary as input,
 * load it, then tries to execute it.
 */

#include <sys/mman.h>
#include <sys/stat.h>

#include <elf.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "simutil.h"

#include "shrmem.h"
#include "dstbmem.h"

#include "cpu.h"
#include "mem.h"
#include "datapath.h"
#include "net.h"

#ifndef IS_ELF
#define IS_ELF(eh) ((eh).e_ident[EI_MAG0] == ELFMAG0 && \
                    (eh).e_ident[EI_MAG1] == ELFMAG1 && \
                    (eh).e_ident[EI_MAG2] == ELFMAG2 && \
                    (eh).e_ident[EI_MAG3] == ELFMAG3)
#endif

int
main(int argc, char *argv[])
{
	int             c;
	int             fd;
	int             flag;

	size_t          memsz;
	size_t          ncpu;
	size_t          x, y;

	Elf32_Ehdr     *eh;

	struct ProgInfo prog;

	struct stat     stat;

	/*
	 * Default options
	 */
	memsz = (1 << 24);	/* 16 Mib */
	ncpu = 1;
	flag = NONE;
	x = y = 1;

	/*
	 * Parse command line options
	 */
	while ((c = getopt(argc, argv, ":n:m:c:")) != -1) {
		switch (c) {
		case 'c':
			flag = SHRMEM;
			ncpu = strtoul(optarg, NULL, 10);
			break;
		case 'n':
			flag = NET;
			x = strtoul(strtok(optarg, "x"), NULL, 10);
			y = strtoul(strtok(NULL, "x"), NULL, 10);
			break;
		case 'm':
			memsz = strtoul(optarg, NULL, 10);
			break;
		case ':':
			errx(EXIT_FAILURE, "Option -%c requires an operand",
			     optopt);
			break;
		case '?':
			errx(EXIT_FAILURE, "Unrecognized option: '-%c'",
			     optopt);
			break;
		}
	}

	prog.name = argv[optind];

	if (flag == NONE) {
		errx(EXIT_FAILURE, "need to pass a mode of operation");
	}

	/*
	 * Read file
	 */
	if (!prog.name) {
		errx(EXIT_FAILURE,
		     "usage: %s (-c N | -n NxM) /path/to/objfile", argv[0]);
	}

	if ((fd = open(prog.name, O_RDONLY)) < 0) {
		err(EXIT_FAILURE, "open: %s", prog.name);
	}

	if (fstat(fd, &stat) < 0) {
		err(EXIT_FAILURE, "fstat");
	}

	prog.size = stat.st_size;
	if ((prog.elf = mmap(NULL, stat.st_size, PROT_READ,
			     MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
		err(EXIT_FAILURE, "mmap");
	}

	/* Checks file type */
	eh = (Elf32_Ehdr *) prog.elf;
	prog.entry = eh->e_entry;
	if (!IS_ELF(*eh) || eh->e_ident[EI_CLASS] != ELFCLASS32 ||
	    eh->e_ident[EI_DATA] != ELFDATA2LSB || eh->e_type != ET_EXEC ||
	    eh->e_machine != EM_MIPS) {
		errx(EXIT_FAILURE, "%s is not a MIPS32 EL executable",
		     prog.name);
	}

	if (flag == SHRMEM) {
		ShrMem_run(ncpu, memsz, &prog);
	} else { 
		DstbMem_run(x, y, memsz, &prog);
	}

	if (close(fd) < 0) {
		err(EXIT_FAILURE, "close");
	}

	return EXIT_SUCCESS;
}

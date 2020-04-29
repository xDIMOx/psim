/* Check LICENSE file for copyright and license details. */

/*
 * MIPS32 simulator, it receives a little-endian MIPS32 binary as input,
 * load it, then tries to execute it.
 */

#include <sys/mman.h>
#include <sys/stat.h>

#include <elf.h>
#include <err.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cpu.h"
#include "mem.h"
#include "datapath.h"

#define IS_ELF(eh) ((eh).e_ident[EI_MAG0] == ELFMAG0 && \
                    (eh).e_ident[EI_MAG1] == ELFMAG1 && \
                    (eh).e_ident[EI_MAG2] == ELFMAG2 && \
                    (eh).e_ident[EI_MAG3] == ELFMAG3)

int
main(int argc, char *argv[])
{
	int             c;
	int             fd;

	size_t          memsz;

	unsigned char  *elf;

	Elf32_Ehdr     *eh;

	CPU            *cpu;

	Mem            *mem;

	struct stat     stat;

	/*
	 * Default options
	 */
	memsz = (1 << 24);	/* 16 Mib */

	/*
	 * Parse command line options
	 */
	while ((c = getopt(argc, argv, ":m:")) != -1) {
		switch (c) {
		case 'm':
			memsz = atol(optarg);
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

	/*
	 * Read file
	 */
	if (!(argv[optind]))
		errx(EXIT_FAILURE, "usage: %s /path/to/objfile", argv[0]);

	if ((fd = open(argv[optind], O_RDONLY)) < 0)
		err(EXIT_FAILURE, "open: %s", argv[optind]);

	if (fstat(fd, &stat) < 0)
		err(EXIT_FAILURE, "fstat");

	if ((elf = mmap(NULL, stat.st_size * sizeof(unsigned char), PROT_READ,
			MAP_PRIVATE, fd, 0)) == MAP_FAILED)
		err(EXIT_FAILURE, "mmap");

	/* Checks file type */
	eh = (Elf32_Ehdr *) elf;
	if (!IS_ELF(*eh) || eh->e_ident[EI_CLASS] != ELFCLASS32 ||
	    eh->e_ident[EI_DATA] != ELFDATA2LSB || eh->e_type != ET_EXEC ||
	    eh->e_machine != EM_MIPS)
		errx(EXIT_FAILURE, "%s is not a MIPS32 EL executable",
		     argv[optind]);

	/*
	 * Component creation
	 */
	if (!(cpu = CPU_create(0)))
		errx(EXIT_FAILURE, "CPU_create: %s", CPU_strerror(CPU_errno));

	if (!(mem = Mem_create(memsz)))
		errx(EXIT_FAILURE, "Mem_create: %s", Mem_strerror(Mem_errno));

	/*
	 * Program loading
	 */
	if (Mem_progld(mem, elf) < 0)
		errx(EXIT_FAILURE, "Mem_progld: %s", Mem_strerror(Mem_errno));

	cpu->pc = eh->e_entry;

	if (munmap(elf, stat.st_size * sizeof(unsigned char)) < 0)
		err(EXIT_FAILURE, "munmap");

	if (close(fd) < 0)
		err(EXIT_FAILURE, "close");

	/*
	 * Program execution
	 */
	while (!Datapath_execute(cpu, mem))
		fflush(stdout);

	if (Datapath_errno != DATAPATHERR_SUCC) {
		errx(EXIT_FAILURE, "Datapath_execute %s",
		     Datapath_strerror(Datapath_errno));
	}

	Mem_destroy(mem);
	CPU_destroy(cpu);

	return EXIT_SUCCESS;
}

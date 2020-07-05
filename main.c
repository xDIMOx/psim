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
#include <libgen.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cpu.h"
#include "mem.h"
#include "datapath.h"

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

	size_t          i;
	size_t          memsz;
	size_t          ncpu;

	char           *prog;

	unsigned char  *elf;

	Elf32_Ehdr     *eh;

	CPU           **cpus;

	Mem            *mem;

	struct stat     stat;

	char            perfct[NAME_MAX];

	/*
	 * Default options
	 */
	memsz = (1 << 24);	/* 16 Mib */
	ncpu = 1;

	/*
	 * Parse command line options
	 */
	while ((c = getopt(argc, argv, ":m:c:")) != -1) {
		switch (c) {
		case 'c':
			ncpu = strtoul(optarg, NULL, 10);
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

	prog = argv[optind];

	/*
	 * Read file
	 */
	if (!prog)
		errx(EXIT_FAILURE, "usage: %s /path/to/objfile", argv[0]);

	if ((fd = open(prog, O_RDONLY)) < 0)
		err(EXIT_FAILURE, "open: %s", prog);

	if (fstat(fd, &stat) < 0)
		err(EXIT_FAILURE, "fstat");

	if ((elf = mmap(NULL, stat.st_size, PROT_READ,
			MAP_PRIVATE, fd, 0)) == MAP_FAILED)
		err(EXIT_FAILURE, "mmap");

	/* Checks file type */
	eh = (Elf32_Ehdr *) elf;
	if (!IS_ELF(*eh) || eh->e_ident[EI_CLASS] != ELFCLASS32 ||
	    eh->e_ident[EI_DATA] != ELFDATA2LSB || eh->e_type != ET_EXEC ||
	    eh->e_machine != EM_MIPS)
		errx(EXIT_FAILURE, "%s is not a MIPS32 EL executable", prog);

	/*
	 * Component creation
	 */
	if (!(cpus = malloc(sizeof(CPU *) * ncpu)))
		err(EXIT_FAILURE, "could not allocate cpu array");

	for (i = 0; i < ncpu; ++i) {
		if (!(cpus[i] = CPU_create(i))) {
			errx(EXIT_FAILURE, "cpu[%lu] -- CPU_create: %s",
			     i, CPU_strerror(CPU_errno));
		}
	}

	if (!(mem = Mem_create(memsz, ncpu)))
		errx(EXIT_FAILURE, "Mem_create: %s", Mem_strerror(Mem_errno));

	/*
	 * Program loading
	 */
	if (Mem_progld(mem, elf) < 0)
		errx(EXIT_FAILURE, "Mem_progld: %s", Mem_strerror(Mem_errno));

	for (i = 0; i < ncpu; ++i)
		cpus[i]->pc = eh->e_entry;

	if (munmap(elf, stat.st_size) < 0)
		err(EXIT_FAILURE, "munmap");

	if (close(fd) < 0)
		err(EXIT_FAILURE, "close");

	/*
	 * Program execution
	 */
	for (i = 0; !Datapath_execute(cpus[i], mem); i = (i + 1) % ncpu) {
		if (i == (ncpu - 1)) {
			Mem_busclr();
			fflush(stdout);
		}
	}

	if (Datapath_errno != DATAPATHERR_EXIT) {
		errx(EXIT_FAILURE, "cpu[%lu] -- Datapath_execute %s",
		     i, Datapath_strerror(Datapath_errno));
	}

	Mem_destroy(mem);

	sprintf(perfct, "./perfct_%s", basename(prog));
	if ((fd = open(perfct, O_WRONLY | O_CREAT | O_TRUNC,
		       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
		err(EXIT_FAILURE, "open: %s", perfct);

	dprintf(fd, "#id,cycles,"
		"loads,ld defer,"
		"stores,st defer,"
		"ll,ll defer,"
		"sc,sc defer,"
		"rmwfail,"
		"ct0,ct1,ct2\n");
	dprintf(fd,"bus,%lu,,,,,,,,,,,,\n", Mem_busutil());
	for (i = 0; i < ncpu; ++i) {
		dprintf(fd,"%u,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",
		      cpus[i]->gpr[K0], cpus[i]->perfct.cycle,
		      cpus[i]->perfct.ld, cpus[i]->perfct.lddefer,
		      cpus[i]->perfct.st, cpus[i]->perfct.stdefer,
		      cpus[i]->perfct.ll, cpus[i]->perfct.lldefer,
		      cpus[i]->perfct.sc, cpus[i]->perfct.scdefer,
		      cpus[i]->perfct.rmwfail,
		      cpus[i]->perfct.ct0, cpus[i]->perfct.ct1,
		      cpus[i]->perfct.ct2);
		CPU_destroy(cpus[i]);
	}

	if (close(fd) < 0)
		err(EXIT_FAILURE, "close");

	return EXIT_SUCCESS;
}

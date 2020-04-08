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
#include <stdlib.h>
#include <unistd.h>

#include "cpu.h"
#include "mem.h"

#define IS_ELF(eh) ((eh).e_ident[EI_MAG0] == ELFMAG0 && \
                    (eh).e_ident[EI_MAG1] == ELFMAG1 && \
                    (eh).e_ident[EI_MAG2] == ELFMAG2 && \
                    (eh).e_ident[EI_MAG3] == ELFMAG3)

int
main(int argc, char *argv[])
{
	int             fd;

	Elf32_Ehdr     *eh;

	CPU            *cpu;

	Mem            *mem;

	/*
	 * Read file
	 */
	if (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s /path/to/objfile", argv[0]);

	if ((fd = open(argv[1], O_RDONLY)) < 0)
		err(EXIT_FAILURE, "open: %s", argv[1]);

	if ((eh = mmap(NULL, sizeof(Elf32_Ehdr), PROT_READ, MAP_PRIVATE,
		       fd, 0)) == MAP_FAILED)
		err(EXIT_FAILURE, "mmap");

	/* Checks file type */
	if (!IS_ELF(*eh) || eh->e_ident[EI_CLASS] != ELFCLASS32 ||
	    eh->e_ident[EI_DATA] != ELFDATA2LSB || eh->e_type != ET_EXEC ||
	    eh->e_machine != EM_MIPS)
		errx(EXIT_FAILURE, "%s is not a MIPS32 EL executable",
		     argv[1]);

	if (munmap(eh, sizeof(Elf32_Ehdr)) < 0)
		err(EXIT_FAILURE, "munmap");

	if (close(fd) < 0)
		err(EXIT_FAILURE, "close");

	if (!(cpu = CPU_create()))
		err(EXIT_FAILURE, "CPU_create");

	if (!(mem = Mem_create(1024)))
		err(EXIT_FAILURE, "Mem_create");

	Mem_destroy(mem);
	CPU_destroy(cpu);

	return EXIT_SUCCESS;
}

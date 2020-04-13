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
#include <string.h>
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
	uint16_t        phnum;

	int             fd;

	void           *file;

	Elf32_Ehdr     *eh;
	Elf32_Phdr     *ph;

	CPU            *cpu;

	Mem            *mem;

	struct stat     stat;

	/*
	 * Read file
	 */
	if (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s /path/to/objfile", argv[0]);

	if ((fd = open(argv[1], O_RDONLY)) < 0)
		err(EXIT_FAILURE, "open: %s", argv[1]);

	if (fstat(fd, &stat) < 0)
		err(EXIT_FAILURE, "fstat");

	if ((file = mmap(NULL, stat.st_size * sizeof(unsigned char), PROT_READ,
			 MAP_PRIVATE, fd, 0)) == MAP_FAILED)
		err(EXIT_FAILURE, "mmap");

	/* Checks file type */
	eh = file;
	if (!IS_ELF(*eh) || eh->e_ident[EI_CLASS] != ELFCLASS32 ||
	    eh->e_ident[EI_DATA] != ELFDATA2LSB || eh->e_type != ET_EXEC ||
	    eh->e_machine != EM_MIPS)
		errx(EXIT_FAILURE, "%s is not a MIPS32 EL executable",
		     argv[1]);

	/*
	 * Program loading
	 */
	if (!(cpu = CPU_create()))
		errx(EXIT_FAILURE, "CPU_create: %s", CPU_strerror(CPU_errno));

	if (!(mem = Mem_create(1024)))
		errx(EXIT_FAILURE, "Mem_create: %s", Mem_strerror(Mem_errno));

	for (ph = (Elf32_Phdr *) file + eh->e_phoff, phnum = eh->e_phnum;
	     phnum > 0;
	     phnum--, ph += eh->e_phentsize) {
		if (ph->p_type != PT_LOAD)
			continue;

		switch (ph->p_flags) {
		case PF_R + PF_X:
		case PF_R + PF_W + PF_X:
			if (ph->p_paddr >= mem->size)
				errx(EXIT_FAILURE,
				     "Program loading: insufficient memory");

			memcpy(mem->data.b + ph->p_paddr,
			     (uint8_t *) file + ph->p_offset, ph->p_filesz);
		}
	}

	cpu->pc = eh->e_entry;

	if (munmap(file, stat.st_size * sizeof(unsigned char)) < 0)
		err(EXIT_FAILURE, "munmap");

	if (close(fd) < 0)
		err(EXIT_FAILURE, "close");

	Mem_destroy(mem);
	CPU_destroy(cpu);

	return EXIT_SUCCESS;
}

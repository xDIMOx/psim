/* Check LICENSE file for copyright and license details. */

/*
 * Memory module
 */

#include <elf.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Implements */
#include "mem.h"

#define X(a, b) b,
static const char *Mem_errlist[] = {
	MemErrList
};
#undef X

Mem            *Mem_create(size_t size);
void            Mem_destroy(Mem *mem);

int             Mem_progld(Mem *mem, unsigned char *elf);

const char     *Mem_strerror(int errno);

/*
 * Mem_create: create memory object
 *
 * size: memory size in bytes
 *
 * Returns a pointer to the newly allocated memory, NULL if failure
 */
Mem            *
Mem_create(size_t size)
{
	Mem            *mem;

	if (!(mem = malloc(sizeof(Mem) * size)))
		return NULL;

	return mem;
}

/*
 * Mem_destroy: free memory object
 */
void
Mem_destroy(Mem *mem)
{
	free(mem);
}

/*
 * Mem_progld: loads program to memory
 *
 * mem: pointer to destination memory
 * elf: ELF image
 *
 * Returns 0 if success, -1 otherwise, Mem_errno indicates the error
 */
int
Mem_progld(Mem *mem, unsigned char *elf)
{
	uint16_t phnum;

	Elf32_Ehdr *eh;
	Elf32_Phdr *ph;

	Mem_errno = MEMERR_SUCC;

	memset(mem->data.b, 0, mem->size);

	eh = (Elf32_Ehdr *) elf;

	for (ph = (Elf32_Phdr *) elf + eh->e_phoff, phnum = eh->e_phnum;
	phnum > 0; --phnum, ++ph) {
		if (ph->p_type != PT_LOAD)
			continue;

		switch (ph->p_flags) {
		case PF_R + PF_X:
		case PF_R + PF_W + PF_X:
			if (ph->p_paddr >= mem->size) {
				Mem_errno = MEMERR_BND;
				return -1;
			}
			memcpy(mem->data.b + ph->p_paddr,
			     (uint8_t *) elf + ph->p_offset, ph->p_filesz);
		}
	}

	return 0;
}

/*
 * Mem_strerror: Map error number to error message string
 *
 * errno: error number
 *
 * Returns error message string
 */
inline const char *
Mem_strerror(int errno)
{
	return Mem_errlist[errno];
}

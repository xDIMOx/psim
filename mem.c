/* Check LICENSE file for copyright and license details. */

/*
 * Memory module
 */

#include <elf.h>
#include <err.h>
#include <errno.h>
#include <pthread.h>
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

static int      shared;

static struct {
	long            used;	/* memory controller was used in the cycle */
	size_t          nshr;	/* no. of processors sharing the bus */
	ssize_t        *resaddr;/* reserved addresses */
	pthread_mutex_t lock;
}               memctl;


Mem            *Mem_create(size_t size, size_t shr);
void            Mem_destroy(Mem *mem);

int             Mem_busacc(void);
void            Mem_busclr(void);

int             Mem_progld(Mem *mem, unsigned char *elf);

int64_t         Mem_lw(Mem *mem, size_t addr);
int             Mem_sw(Mem *mem, size_t addr, uint32_t data);

int64_t         Mem_lb(Mem *mem, size_t addr);
int             Mem_sb(Mem *mem, size_t addr, uint8_t data);

const char     *Mem_strerror(int errno);

/*
 * Mem_create: create memory object
 *
 * size: memory size in bytes
 * nshr: number of processors sharing the memory
 *
 * Returns a pointer to the newly allocated memory, NULL if failure
 */
Mem            *
Mem_create(size_t size, size_t nshr)
{
	Mem            *mem;

	Mem_errno = MEMERR_SUCC;

	if (nshr > 1) {
		shared = 1;
		memctl.used = 0;
		memctl.nshr = nshr;
		if (!(memctl.resaddr = malloc(sizeof(ssize_t) * nshr)) ||
		    pthread_mutex_init(&memctl.lock, NULL)) {
			Mem_errno = MEMERR_ALLOC;
			return NULL;
		}
	}

	if (!(mem = malloc(sizeof(Mem))) ||
	    !(mem->data.b = malloc(size * sizeof(uint8_t)))) {
		Mem_errno = MEMERR_ALLOC;
		return NULL;
	}

	mem->size = size;

	return mem;
}

/*
 * Mem_destroy: free memory object
 */
void
Mem_destroy(Mem *mem)
{
	if (shared) {
		pthread_mutex_destroy(&memctl.lock);
		free(memctl.resaddr);
	}

	free(mem);
}

/*
 * busacc: try to access the memory bus, setting the state of the memory
 *         controller
 *
 * Returns 0 if successfully acquired the bus, an error number otherwise
 */
int
Mem_busacc(void)
{
	int             errcode;

	errcode = 0;
	if (shared) {
		if ((errcode = pthread_mutex_trylock(&memctl.lock)) &&
		    (errcode != EBUSY)) {
			warnx("Mem_busacc -- pthread_mutex_trylock: %s",
			      strerror(errcode));
		} else if (memctl.used) {
			errcode = EBUSY;
		} else
			memctl.used = 1;
		pthread_mutex_unlock(&memctl.lock);
	}

	return errcode;
}

/*
 * Mem_busclr: clear the state on the memory controller
 */
void 
Mem_busclr(void)
{
	memctl.used = 0;
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
	uint16_t        phnum;

	Elf32_Ehdr     *eh;
	Elf32_Phdr     *ph;

	Mem_errno = MEMERR_SUCC;

	memset(mem->data.b, 0, mem->size);

	eh = (Elf32_Ehdr *) elf;

	for (ph = (Elf32_Phdr *) (elf + eh->e_phoff), phnum = eh->e_phnum;
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
 * Mem_lw: load word
 *
 * mem: pointer to memory
 * addr: address where the data is
 *
 * Returns data if successful, -1 otherwise, Mem_errno indicates the error
 */
int64_t
Mem_lw(Mem *mem, size_t addr)
{
	Mem_errno = MEMERR_SUCC;

	if (addr >= mem->size) {
		Mem_errno = MEMERR_BND;
		return -1;
	}
	if (addr & 3) {
		Mem_errno = MEMERR_ALIGN;
		return -1;
	}
	return mem->data.w[addr >> 2];
}

/*
 * Mem_sw: store word
 *
 * mem: Pointer to memory
 * addr: Address where the data is;
 * data: Data to be stored
 *
 * Returns 0 if successful, -1 otherwise, Mem_errno indicates the error
 */
int
Mem_sw(Mem *mem, size_t addr, uint32_t data)
{
	Mem_errno = MEMERR_SUCC;

	if (addr >= mem->size) {
		Mem_errno = MEMERR_BND;
		return -1;
	}
	if (addr & 3) {
		Mem_errno = MEMERR_ALIGN;
		return -1;
	}
	mem->data.w[addr >> 2] = data;

	return 0;
}

/*
 * Mem_lb: Load byte
 *
 * addr: Address where the data is;
 *
 * Returns the data if success, -1 otherwise
 */
int64_t
Mem_lb(Mem *mem, size_t addr)
{
	Mem_errno = MEMERR_SUCC;

	if (addr >= mem->size) {
		Mem_errno = MEMERR_BND;
		return -1;
	}
	return mem->data.b[addr];
}

/*
 * Mem_sb: Store byte
 *
 * addr: Address where the data is;
 * data: Data to be stored
 */
int
Mem_sb(Mem *mem, size_t addr, uint8_t data)
{
	Mem_errno = MEMERR_SUCC;

	if (addr >= mem->size) {
		Mem_errno = MEMERR_BND;
		return -1;
	}
	mem->data.b[addr] = data;

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

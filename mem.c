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

static int      shared;

static struct {
	long            used;		/*
					 * memory controller was used in the
					 * cycle
					 */
	size_t          nshr;		/* no. of processors sharing the bus */
	ssize_t        *resaddr;	/* reserved addresses */
	struct {
		size_t          hd, tl;
		ssize_t        *arr;
	}               queue;
	size_t          util;
	pthread_mutex_t lock;
}               memctl;


Mem            *Mem_create(size_t, size_t);
void            Mem_destroy(Mem *);

int             Mem_busacc(uint32_t);
void            Mem_busclr(void);
size_t          Mem_busutil(void);

int             Mem_progld(Mem *, unsigned char *);

int64_t         Mem_lw(Mem *, size_t);
int             Mem_sw(Mem *, size_t, uint32_t);

int64_t         Mem_ll(Mem *, uint32_t prid, size_t);
int             Mem_sc(Mem *, uint32_t prid, size_t, uint32_t);

int64_t         Mem_lb(Mem *, size_t);
int             Mem_sb(Mem *, size_t, uint8_t);

uint32_t       *Mem_getptr(Mem *, uint32_t);

const char     *Mem_strerror(int);

/*
 * Mem_create: create memory object
 *
 * size: memory size in bytes
 * nshr: number of processors sharing the memory
 *
 * Returns a pointer to the newly allocated memory object, NULL if failure. In
 * case of failure errno indicates the error.
 *
 * The function fails if:
 *	ENOMEM: Could not allocate memory object.
 */
Mem *
Mem_create(size_t size, size_t nshr)
{
	size_t          i;

	Mem            *mem;

	errno = 0;

	if (nshr > 1) {
		shared = 1;
		memctl.used = 0;
		memctl.nshr = nshr;
		if (!(memctl.resaddr = malloc(sizeof(ssize_t) * nshr)) ||
		    !(memctl.queue.arr = malloc(sizeof(ssize_t) * nshr)) ||
		    pthread_mutex_init(&memctl.lock, NULL)) {
			warn("Mem_create -- shared processors initialization");
			errno = ENOMEM;
			return NULL;
		}
		memctl.queue.hd = memctl.queue.tl = 0;
		memctl.util = 0;
		for (i = 0; i < memctl.nshr; ++i) {
			memctl.queue.arr[i] = -1;
		}
	}

	if (!(mem = malloc(sizeof(Mem))) ||
	    !(mem->data.b = malloc(size * sizeof(uint8_t)))) {
		errno = ENOMEM;
		return NULL;
	}

	mem->size = size;

	return mem;
}

/*
 * Mem_destroy: free memory object
 *
 * mem: memory object
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
 * Mem_busacc: try to access the memory bus, setting the state of the memory
 *             controller
 *
 * prid: processor id
 *
 * Returns 0 if successfully acquired the bus, -1 otherwise.
 */
int
Mem_busacc(uint32_t prid)
{
	int             errcode;
	int             found;

	size_t          i;

	if (!shared) {
		return 0;
	}

	errcode = 0;

	if ((errcode = pthread_mutex_lock(&memctl.lock))) {
		warnx("Mem_busacc cpu[%u] -- "
		      "pthread_mutex_lock: %s",
		      prid, strerror(errcode));
		return -1;
	}

	/* checks if processor is already waiting to use the memory */
	for (found = i = 0; !found && i < memctl.nshr; ++i) {
		if (memctl.queue.arr[i] == prid)
			found = 1;
	}

	/* enqueue if not present */
	if (!found) {
		memctl.queue.arr[memctl.queue.tl] = prid;
		memctl.queue.tl = (memctl.queue.tl + 1) % memctl.nshr;
	}

	/* dequeue if calling processor is in the queue's head */
	if (!memctl.used &&
	    memctl.queue.arr[memctl.queue.hd] == prid) {
		memctl.queue.arr[memctl.queue.hd] = -1;
		memctl.queue.hd = (memctl.queue.hd + 1) % memctl.nshr;
		memctl.used = 1;
		++memctl.util;
	} else {
		errcode = -1;
	}

	pthread_mutex_unlock(&memctl.lock);

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
 * Mem_busutil: returns bus utilization
 */
size_t
Mem_busutil(void)
{
	return memctl.util;
}

/*
 * Mem_progld: loads program to memory
 *
 * mem: memory object
 * elf: ELF image
 *
 * Returns 0 if success, an error number otherwise.
 *
 * This function fails if:
 *	ENOSPC: could not load because the segment is out of bounds.
 */
int
Mem_progld(Mem *mem, unsigned char *elf)
{
	uint16_t        phnum;

	Elf32_Ehdr     *eh;
	Elf32_Phdr     *ph;

	memset(mem->data.b, 0, mem->size);

	eh = (Elf32_Ehdr *) elf;

	for (ph = (Elf32_Phdr *) (elf + eh->e_phoff), phnum = eh->e_phnum;
	     phnum > 0; --phnum, ++ph) {
		if (ph->p_type != PT_LOAD) {
			continue;
		}

		switch (ph->p_flags) {
		case PF_R | PF_X:
		case PF_R | PF_W:
		case PF_R | PF_W | PF_X:
			if (ph->p_paddr >= mem->size) {
				return ENOSPC;
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
 * mem:  memory object
 * addr: address where the data is
 *
 * Returns data if successful, -1 otherwise. In case of failure errno indicates
 * the error.
 *
 * This function fails if:
 *	EADDRNOTAVAIL: address is out of bounds.
 *	EFAULT: address is not word aligned.
 */
int64_t
Mem_lw(Mem *mem, size_t addr)
{
	errno = 0;

	if (addr >= mem->size) {
		errno = EADDRNOTAVAIL;
		return -1;
	}

	if (addr & 3) {
		errno = EFAULT;
		return -1;
	}

	return mem->data.w[addr >> 2];
}

/*
 * Mem_sw: store word
 *
 * mem:  memory object
 * addr: Address where the data is
 * data: Data to be stored
 *
 * Returns 0 if successful, an error code otherwise.
 *
 * This function fails if:
 *	EADDRNOTAVAIL: address is out of bounds.
 *	EFAULT: address is not word aligned.
 */
int
Mem_sw(Mem *mem, size_t addr, uint32_t data)
{
	uint32_t        i;

	errno = 0;

	if (shared) {
		for (i = 0; i < memctl.nshr; ++i) {
			if (memctl.resaddr[i] == (ssize_t) addr)
				memctl.resaddr[i] = -1;
		}
	}

	if (addr >= mem->size) {
		return EADDRNOTAVAIL;
	}

	if (addr & 3) {
		return EFAULT;
	}

	mem->data.w[addr >> 2] = data;

	return 0;
}

/*
 * Mem_ll: load linked
 *
 * mem:  memory object
 * prid: id of the processor doing the operation
 * addr: address where the data is
 *
 * Returns data if successful, -1 otherwise. In case of failure errno indicates
 * the error.
 *
 * This function fails if:
 *	EOVERFLOW: tried to access reserved addresses array out of bounds.
 *	EADDRNOTAVAIL: address is out of bounds.
 *	EFAULT: address is not word aligned.
 */
int64_t
Mem_ll(Mem *mem, uint32_t prid, size_t addr)
{
	errno = 0;

	if (shared) {
		if (prid >= memctl.nshr) {
			errno = EOVERFLOW;
			return -1;
		}
		memctl.resaddr[prid] = addr;
	}

	if (addr >= mem->size) {
		errno = EADDRNOTAVAIL;
		return -1;
	}

	if (addr & 3) {
		errno = EFAULT;
		return -1;
	}

	return mem->data.w[addr >> 2];
}

/*
 * Mem_sc: store word
 *
 * mem:  memory object
 * prid: id of the processor doing the operation
 * addr: Address where the data is;
 * data: Data to be stored
 *
 * Returns 0 if successful, an error number otherwise.
 *
 * This function fails if:
 *	EOVERFLOW: tried to access reserved addresses array out of bounds.
 *	EAGAIN: SC failed.
 *	EADDRNOTAVAIL: address is out of bounds.
 *	EFAULT: address is not word aligned.
 */
int
Mem_sc(Mem *mem, uint32_t prid, size_t addr, uint32_t data)
{
	uint32_t        i;

	errno = 0;

	if (shared) {
		if (prid >= memctl.nshr) {
			return EOVERFLOW;
		}
		if (memctl.resaddr[prid] != (ssize_t) addr) {
			return EAGAIN;
		}
		for (i = 0; i < memctl.nshr; ++i) {
			if (memctl.resaddr[i] == (ssize_t) addr)
				memctl.resaddr[i] = -1;
		}
	}

	if (addr >= mem->size) {
		return EADDRNOTAVAIL;
	}

	if (addr & 3) {
		return EFAULT;
	}

	mem->data.w[addr >> 2] = data;

	return 0;
}

/*
 * Mem_lb: Load byte
 *
 * mem:  memory object
 * addr: address where the data is;
 *
 * Returns the data if success, -1 otherwise. In case of failure errno
 * indicates the error.
 *
 * This function fails if:
 *	EADDRNOTAVAIL: address is out of bounds.
 */
int64_t
Mem_lb(Mem *mem, size_t addr)
{
	errno = 0;

	if (addr >= mem->size) {
		errno = EADDRNOTAVAIL;
		return -1;
	}

	return mem->data.b[addr];
}

/*
 * Mem_sb: Store byte
 *
 * mem:  memory object
 * addr: address where the data is;
 * data: data to be stored
 *
 * Returns 0 if success, an error number otherwise.
 *
 * This function fails if:
 *	EADDRNOTAVAIL: address is out of bounds.
 */
int
Mem_sb(Mem *mem, size_t addr, uint8_t data)
{
	errno = 0;

	if (addr >= mem->size) {
		return EADDRNOTAVAIL;
	}

	mem->data.b[addr] = data;

	return 0;
}

/*
 * Mem_getptr: Get a pointer to the simulation memory
 *
 * mem:  memory object
 * addr: address to get pointer from
 *
 * Returns pointer to simulation memory at address 'addr'. In case of failure
 * errno indicates the error.
 *
 * This function fails if:
 *	EADDRNOTAVAIL: address is out of bounds.
 */
uint32_t *
Mem_getptr(Mem *mem, uint32_t addr)
{
	errno = 0;

	if (addr >= mem->size) {
		errno = EADDRNOTAVAIL;
		return NULL;
	}

	return &(mem->data.w[addr >> 2]);
}

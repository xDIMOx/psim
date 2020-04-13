/* Check LICENSE file for copyright and license details. */

/*
 * Memory module
 */

#include <stdint.h>
#include <stdlib.h>

/* Implements */
#include "mem.h"

#define X(a, b) b,
static const char *Mem_errlist[] = {
	MemErrList
};
#undef X

Mem            *Mem_create(size_t size);
void            Mem_destroy(Mem *mem);

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

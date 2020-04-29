/* Check LICENSE file for copyright and license details. */

/*
 * CPU module
 *
 * Implementation a MIPS processor
 */

#include <stdint.h>
#include <stdlib.h>

#ifndef NDEBUG
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#endif

/* Implements */
#include "cpu.h"

#define X(a, b) b,
static const char *CPU_errlist[] = {
	CPUErrList
};
#undef X

CPU            *CPU_create(uint32_t id);
void            CPU_destroy(CPU *cpu);

const char     *CPU_strerror(int errno);

/*
 * CPU_create: create CPU object
 *
 * id: CPU id
 *
 * Returns cpu if success, NULL otherwise
 */
CPU            *
CPU_create(uint32_t id)
{
	CPU            *cpu;

	CPU_errno = CPUERR_SUCC;

	if (!(cpu = malloc(sizeof(CPU)))) {
		CPU_errno = CPUERR_ALLOC;
		return NULL;
	}
	cpu->gpr[K0] = id;

#ifndef NDEBUG
	snprintf(cpu->debug.fname, 20, "cpu%04d_instrdump", id);
	if ((cpu->debug.fd = open(cpu->debug.fname,
				  O_CREAT | O_WRONLY | O_TRUNC,
				  S_IRUSR | S_IWUSR)) < 0) {
		warn("cpu id = %u -- open: %s", id, cpu->debug.fname);
		CPU_errno = CPUERR_DEBUG;
		return NULL;
	}
#endif

	return cpu;
}

/*
 * CPU_destroy: free memory object
 */
void
CPU_destroy(CPU *cpu)
{
#ifndef NDEBUG
	close(cpu->debug.fd);
#endif
	free(cpu);
}

/*
 * CPU_strerror: Map error number to error message string
 *
 * errno: error number
 *
 * Returns error message string
 */
inline const char *
CPU_strerror(int errno)
{
	return CPU_errlist[errno];
}

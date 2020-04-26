/* Check LICENSE file for copyright and license details. */

/*
 * CPU module
 *
 * Implementation a MIPS processor
 */

#include <stdint.h>
#include <stdlib.h>

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

	return cpu;
}

/*
 * CPU_destroy: free memory object
 */
void
CPU_destroy(CPU *cpu)
{
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

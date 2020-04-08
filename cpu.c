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

CPU            *CPU_create(void);
void            CPU_destroy(CPU *cpu);

/*
 * CPU_create: create CPU object
 */
CPU            *
CPU_create(void)
{
	CPU            *cpu;

	if (!(cpu = malloc(sizeof(CPU))))
		return NULL;

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

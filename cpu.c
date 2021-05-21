/* Check LICENSE file for copyright and license details. */

/*
 * CPU module
 *
 * Implementation a MIPS processor
 */

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef INSTRDUMP
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#endif

/* Implements */
#include "cpu.h"

CPU            *CPU_create(uint32_t);
void            CPU_destroy(CPU *);

void            CPU_setpc(CPU *, uint32_t);

int64_t         CPU_mfc2(CPU *, uint32_t, uint32_t);
int             CPU_mtc2(CPU *, uint32_t, uint32_t, uint32_t);

/*
 * CPU_create: create CPU object
 *
 * id: CPU id
 *
 * Returns cpu if success, NULL otherwise, errno indicates the error.
 *
 * This function fails if:
 *	ENOMEM: Could not allocate CPU.
 *	ENOENT: Could not create debug file (if debuging is enabled).
 */
CPU *
CPU_create(uint32_t id)
{
	size_t          i;

	CPU            *cpu;

	errno = 0;

	if (!(cpu = malloc(sizeof(CPU)))) {
		errno = ENOMEM;
		return NULL;
	}

	cpu->running = 1;
	cpu->gpr[K0] = id;
	cpu->perfct.cycle = 0;
	cpu->perfct.ld = cpu->perfct.lddefer = 0;
	cpu->perfct.st = cpu->perfct.stdefer = 0;
	cpu->perfct.ll = cpu->perfct.lldefer = 0;
	cpu->perfct.sc = cpu->perfct.scdefer = 0;
	cpu->perfct.rmwfail = 0;
	cpu->perfct.nin = cpu->perfct.nout = 0;
	cpu->perfct.waitin = cpu->perfct.waitout = cpu->perfct.waitalt = 0;
	for (i = 0; i < NCOUNTERS; ++i) {
		cpu->perfct.ct[i].en = cpu->perfct.ct[i].ct = 0;
	}

#ifdef INSTRDUMP
	snprintf(cpu->debug.fname, 20, "cpu%04d_instrdump", id);
	if ((cpu->debug.fd = open(cpu->debug.fname,
				  O_CREAT | O_WRONLY | O_TRUNC,
				  S_IRUSR | S_IWUSR)) < 0) {
		warn("CPU_create -- open %s", cpu->debug.fname);
		errno = ENOENT;
		return NULL;
	}
#endif

	return cpu;
}

/*
 * CPU_destroy: free CPU object
 *
 * cpu: cpu object
 */
void
CPU_destroy(CPU *cpu)
{
#ifdef INSTRDUMP
	if (close(cpu->debug.fd)) {
		warn("CPU_destroy -- close %s", cpu->debug.fname);
	}
#endif
	free(cpu);
}

/*
 * CPU_setpc: set program counter
 *
 * cpu: cpu object
 * pc:  program counter
 */
inline void
CPU_setpc(CPU *cpu, uint32_t pc)
{
	cpu->pc = pc;
}

/*
 * CPU_mfc2: move from coprocessor 2
 *
 * cpu: cpu object
 * src: source register
 * sel: select
 *
 * Returns the contents of copressor 2 register if success, -1 otherwise, errno
 * indicates the error.
 *
 * This function fails if:
 *	EINVAL: Either src or sel are invalid values.
 */
int64_t
CPU_mfc2(CPU *cpu, uint32_t src, uint32_t sel)
{
	errno = 0;

	if (src >= COP2_NREG || sel >= COP2_NSEL) {
		errno = EINVAL;
		return -1;
	}

	return cpu->cop2[src][sel];
}

/*
 * CPU_mtc2: Move to coprocessor 2
 *
 * cpu:  cpu object
 * dest: destination register
 * sel:  select
 * val:  value to insert
 *
 * Returns 0 if success, an error code otherwise.
 *
 * This function fails if:
 *	EINVAL: Either src or sel are invalid values.
 */
int
CPU_mtc2(CPU *cpu, uint32_t dest, uint32_t sel, uint32_t val)
{
	errno = 0;

	if (dest >= COP2_NREG || sel >= COP2_NSEL) {
		return EINVAL;
	}

	cpu->cop2[dest][sel] = val;

	return 0;
}

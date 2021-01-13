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

CPU            *CPU_create(uint32_t);
void            CPU_destroy(CPU *);

void            CPU_setpc(CPU *, uint32_t);

int64_t         CPU_mfc2(CPU *, uint32_t, uint32_t);
int             CPU_mtc2(CPU *, uint32_t, uint32_t, uint32_t);

const char     *CPU_strerror(int);

/*
 * CPU_create: create CPU object
 *
 * id: CPU id
 *
 * Returns cpu if success, NULL otherwise
 */
CPU *
CPU_create(uint32_t id)
{
	size_t          i;

	CPU            *cpu;

	CPU_errno = CPUERR_SUCC;

	if (!(cpu = malloc(sizeof(CPU)))) {
		CPU_errno = CPUERR_ALLOC;
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
	cpu->perfct.commwait = 0;
	for (i = 0; i < NCOUNTERS; ++i) {
		cpu->perfct.ct[i].en = cpu->perfct.ct[i].ct = 0;
	}

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
 * CPU_setpc: set program counter
 *
 * cpu: cpu
 * pc: program counter
 */
inline void
CPU_setpc(CPU *cpu, uint32_t pc)
{
	cpu->pc = pc;
}

/*
 * CPU_mfc2: move from coprocessor 2
 *
 * cpu: cpu
 * src: source register
 * sel: select
 *
 */
int64_t
CPU_mfc2(CPU *cpu,  uint32_t src, uint32_t sel)
{
	CPU_errno = CPUERR_SUCC;

	if (src >= COP2_NREG || sel >= COP2_NSEL) {
		CPU_errno = CPUERR_COP2REG;
		return -1;
	}

	return cpu->cop2[src][sel];
}

/*
 * CPU_mtc2: Move to coprocessor 2
 *
 * cpu: cpu
 * dest: destination register
 * sel: select
 * val: value to insert
 */
int
CPU_mtc2(CPU *cpu, uint32_t dest, uint32_t sel, uint32_t val)
{
	CPU_errno = CPUERR_SUCC;

	if (dest >= COP2_NREG || sel >= COP2_NSEL) {
		CPU_errno = CPUERR_COP2REG;
		return -1;
	}

	cpu->cop2[dest][sel] = val;

	return 0;
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

/* Check LICENSE file for copyright and license details. */

/*
 * Processor's datapath
 */

#include <err.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifndef NDEBUG
#include <unistd.h>
#endif

#include "cpu.h"
#include "mem.h"

/* Implements */
#include "datapath.h"

#define X(a, b) b,
static const char *Datapath_errlist[] = {
	DatapathErrList
};
#undef X

static int64_t  fetch(CPU *cpu, Mem *mem);

int             Datapath_execute(CPU *cpu, Mem *mem);

const char     *Datapath_strerror(int errno);

/*
 * fetch: Fetch an instruction
 *
 * cpu: CPU fetching the instruction
 * mem: memory where the instruction is
 *
 * Returns instruction if success, -1 otherwise
 */
static int64_t
fetch(CPU *cpu, Mem *mem)
{
	int64_t         instr;

	if ((instr = Mem_lw(mem, cpu->pc)) < 0) {
		Datapath_errno = DATAPATHERR_FET;
		warnx("fetch Mem_lw: %s", Mem_strerror(Mem_errno));
		return -1;
	}
	return instr;
}

/*
 * Datapath_execute: execute a cycle
 *
 * cpu: CPU running at the moment
 * mem: memory that the CPU is using
 *
 * Returns 0 if success, -1 otherwise
 */
int
Datapath_execute(CPU *cpu, Mem *mem)
{
	int64_t         instr;

	Datapath_errno = DATAPATHERR_SUCC;

	if ((instr = fetch(cpu, mem)) < 0)
		return -1;

#ifndef NDEBUG
	write(cpu->debug.fd, &instr, sizeof(uint32_t));
#endif

	cpu->pc += 4;

	return 0;
}

/*
 * Datapath_strerror: Map error number to error message string
 *
 * errno: error number
 *
 * Returns error message string
 */
inline const char *
Datapath_strerror(int errno)
{
	return Datapath_errlist[errno];
}

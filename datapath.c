/* Check LICENSE file for copyright and license details. */

/*
 * Processor's datapath
 */

#include <err.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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

int
Datapath_execute(CPU *cpu, Mem *mem)
{
	int64_t         instr;

	Datapath_errno = DATAPATHERR_SUCC;

	if ((instr = fetch(cpu, mem)) < 0)
		return -1;

	printf("%x\n", (uint32_t) instr);

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

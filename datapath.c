/* Check LICENSE file for copyright and license details. */

/*
 * Processor's datapath
 */

#include <stddef.h>
#include <stdint.h>

#include "cpu.h"
#include "mem.h"

/* Implements */
#include "datapath.h"

#define X(a, b) b,
static const char *Datapath_errlist[] = {
	DatapathErrList
};
#undef X

int             Datapath_execute(CPU *cpu, Mem *mem);

const char     *Datapath_strerror(int errno);

int
Datapath_execute(CPU *cpu, Mem *mem)
{
	Datapath_errno = DATAPATHERR_SUCC;

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

/* Check LICENSE file for copyright and license details. */

/*
 * Processor's datapath
 */

/*
 * Error handling
 */

#define DatapathErrList                            \
X(DATAPATHERR_SUCC, "Success")                     \
X(DATAPATHERR_FET, "Could not fetch")              \
X(DATAPATHERR_DEC, "Could not decode")             \
X(DATAPATHERR_RES, "Reserved instruction")         \
X(DATAPATHERR_IMPL, "Instruction not implemented") \
X(DATAPATHERR_EXIT, "Simulation ended")

#define X(a, b) a,
enum DatapathErrNo {
	DatapathErrList
};
#undef X

int             Datapath_errno;

int             Datapath_execute(CPU *cpu, Mem *mem);

const char     *Datapath_strerror(int errno);

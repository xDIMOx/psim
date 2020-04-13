/* Check LICENSE file for copyright and license details. */

/*
 * Processor's datapath
 */

/*
 * Error handling
 */

#define DatapathErrList \
X(DATAPATHERR_SUCC, "Success")

#define X(a, b) a,
enum DatapathErrNo {
	DatapathErrList
};
#undef X

int             Datapath_errno;

int             Datapath_execute(CPU *cpu, Mem *mem);

const char     *Datapath_strerror(int errno);

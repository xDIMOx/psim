/* Check LICENSE file for copyright and license details. */

/*
 * Network node
 */

/*
 * Error handling
 */

#define NetErrList                           \
X(NETERR_SUCC, "Success")                    \
X(NETERR_ALLOC, "Could not allocate memory") \
X(NETERR_CPU, "Could not allocate CPU")      \
X(NETERR_MEM, "Could not allocate simulation memory")

#define X(a, b) a,
enum NetErrNo {
	NetErrList
};
#undef X

/*
 * Definitions
 */

typedef struct {
	size_t          cycle;
	size_t          size;
	struct node {
		CPU            *cpu;
		Mem            *mem;
	}              *nd;
} Net;				/* processor network */

int             Net_errno;

Net            *Net_create(size_t x, size_t y, size_t memsz);
void            Net_destroy(Net *net);

void            Net_setpc(Net *net, size_t id, uint32_t pc);

int             Net_progld(Net *net, size_t memsz, unsigned char *elf);

void            Net_runsim(Net *net);

void            Net_perfct(Net *net, char *progname);

const char     *Net_strerror(int code);

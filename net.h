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

struct msg {
	uint16_t        to;
	uint16_t        from;
	uint32_t        data;
	uint32_t        hops;
	uint32_t        ack;
	struct msg     *nxt;
};

typedef struct {
	size_t          cycle;
	size_t          size;
	size_t          x, y;
	struct node {
		CPU            *cpu;
		Mem            *mem;
	}              *nd;
} Net;				/* processor network */

int             Net_errno;

Net            *Net_create(size_t, size_t, size_t);
void            Net_destroy(Net *);

void            Net_setpc(Net *, size_t, uint32_t);

int             Net_progld(Net *, size_t, unsigned char *);

void            Net_runsim(Net *);

void            Net_perfct(Net *, char *);

const char     *Net_strerror(int);

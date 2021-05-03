/* Check LICENSE file for copyright and license details. */

/*
 * Network node
 */

/*
 * Error handling
 */

#define LinknameList   \
Y(LINK_NORTH, "north") \
Y(LINK_EAST, "east")   \
Y(LINK_SOUTH, "south") \
Y(LINK_WEST, "west")   \
Y(LINK_MBOX, "mbox")

#define X(a, b) a,
enum NetErrNo {
	NetErrList
};
#undef X

#define LINK_BUFSZ 2

#define MAXMSG 1

#define LINK_DIR 2		/* number of link directions */

#define LINK_NAMES 4		/* number of link names */

#define MBOX_BUFSZ 1

enum linkdir {			/* transmission direction */
	LINK_IN,
	LINK_OUT,
};

#define Y(a, b) a,
enum linkname {
	LinknameList
};
#undef Y

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
	size_t          nrun;	/* no. of running processors */
	struct node {
		CPU            *cpu;
		Mem            *mem;
		struct link {
			struct msg     *hd;
			struct msg     *tl;
			size_t          len;
		}               link[LINK_DIR][LINK_NAMES];
		size_t          linkutil[LINK_DIR][LINK_NAMES];
		size_t          mbox_start;	/* for round-robin */
		size_t          mbox_new;	/* new messages */
		struct msg    **mbox;
	}              *nd;
} Net;				/* processor network */

Net            *Net_create(size_t, size_t, size_t);
void            Net_destroy(Net *);

void            Net_setpc(Net *, size_t, uint32_t);

int             Net_progld(Net *, size_t, unsigned char *);

void            Net_runsim(Net *);

void            Net_perfct(Net *, char *);

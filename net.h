/* Check LICENSE file for copyright and license details. */

/*
 * Network implementation
 */

/*
 * Error handling
 */

#define LinkNameList   \
Y(LINK_NORTH, "north") \
Y(LINK_EAST,  "east")  \
Y(LINK_SOUTH, "south") \
Y(LINK_WEST,  "west")  \
Y(LINK_MBOX,  "mbox")

#define LINK_BUFSZ 4

#define MAXMSG 1

#define LINK_DIR 2		/* number of link directions */

#define LINK_NAMES 4		/* number of link names */

#define MBOX_BUFSZ 1

enum LinkDir {			/* transmission direction */
	LINK_IN,
	LINK_OUT,
};

#define Y(a, b) a,
enum LinkName {
	LinkNameList
};
#undef Y

/*
 * Definitions
 */

struct Msg {
	uint16_t        to;
	uint16_t        from;
	uint32_t        data;
	uint32_t        hops;
	uint32_t        ack;
	struct Msg     *nxt;
};

typedef struct {
	size_t          cycle;
	size_t          size;
	size_t          x, y;
	size_t          nrun;	/* no. of running processors */
	struct Node {
		CPU            *cpu;
		Mem            *mem;
		struct Link {
			struct Msg     *hd;
			struct Msg     *tl;
			size_t          len;
		}               link[LINK_DIR][LINK_NAMES];
		size_t          linkutil[LINK_DIR][LINK_NAMES];
		size_t          mbox_start;	/* for round-robin */
		size_t          mbox_new;	/* new messages */
		struct Msg    **mbox;
	}              *nd;
} Net;				/* processor network */

int             Net_execute(Net *);

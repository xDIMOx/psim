/* Check LICENSE file for copyright and license details. */

/*
 * Network node
 */

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cpu.h"
#include "mem.h"
#include "datapath.h"

/* Implements */
#include "net.h"

#define Y(a, b) b,
static const char *linkname[] = {
	LinknameList
};
#undef Y

static int         link_insmsg(struct link *, struct msg *);
static struct msg *link_remmsg(struct link *);

static int      input(Net *, size_t, size_t);
static int      output(Net *, size_t, size_t);
static int      alt(Net *, uint32_t *, size_t);

static int      guidance(Net *, size_t, size_t);
static void     fwd(Net *, size_t);
static void     hop(Net *, uint32_t);
static int      operate(Net *, size_t);

static int      execute(Net *);

Net            *Net_create(size_t, size_t, size_t);
void            Net_destroy(Net *);

void            Net_setpc(Net *, size_t, uint32_t);

int             Net_progld(Net *, size_t, unsigned char *);

void            Net_runsim(Net *);

void            Net_perfct(Net *, char *);

/*
 * link_insmsg: insert message on link
 *
 * link: link to operate
 * msg:  message to insert
 *
 * Returns 0 if successfully inserted on the link's circular buffer, -1
 * otherwise.
 */
static
int
link_insmsg(struct link *link, struct msg *msg)
{
	if (link->len == LINK_BUFSZ) {
		return -1;
	}

	if (link->len > 0) {
		link->tl->nxt = msg;
		msg->nxt = NULL;
		link->tl = msg;
	} else {
		link->hd = msg;
		link->tl = msg;
	}

	++link->len;

	return 0;
}

/*
 * link_remmsg: remove message from link
 *
 * link: link to operate
 *
 * Returns a pointer to the removed message if sucessfull, NULL otherwise.
 */
static
struct msg *
link_remmsg(struct link *link)
{
	struct msg     *msg;

	if (!link->hd) {
		return NULL;
	}

	msg = link->hd;
	link->hd = link->hd->nxt;
	msg->nxt = NULL;
	--link->len;

	return msg;
}

/*
 * input: execute input command
 *
 * net:  network object
 * from: who the message is from
 * to:   who the message is to
 *
 * Returns 0 if succesfull, an error number otherwise.
 *
 * This function fails if:
 *	EAGAIN: message not arrived yet, try later.
 *	ENOMEM: could not allocate an object.
 *	ENOBUFS: link buffer full.
 */
static int
input(Net *net, size_t from, size_t to)
{
	int             link;

	uint32_t        hops;

	struct msg     *msg, *ack;

	struct link    *olink;

	msg = net->nd[to].mbox[from];

	if (!msg) {
		return EAGAIN;
	}

	if (!(ack = malloc(sizeof(*ack)))) {
		warn("%s operate -- nd[%lu] cycle %lu -- "
		      "could not allocate ack (input)",
		      __FILE__, to, net->cycle);
		return ENOMEM;
	}

	ack->to = msg->from;
	ack->from = to;
	ack->ack = 1;
	ack->hops = msg->hops;
	ack->nxt = NULL;

	link = guidance(net, to, ack->to);
	olink = &(net->nd[to].link[LINK_OUT][link]);
	if (olink->len >= LINK_BUFSZ) {
		warnx("%s input -- nd[%lu] cycle %lu -- "
		      "could not send acknowledge (link %s full)",
		      __FILE__, to, net->cycle, linkname[link]);
		return ENOBUFS;
	}

	link_insmsg(olink, ack);

	CPU_mtc2(net->nd[to].cpu, COP2_MSG, COP2_MSG_DATA, msg->data);

	hops = CPU_mfc2(net->nd[to].cpu, COP2_MSG, COP2_MSG_HOPS);
	hops += msg->hops;
	CPU_mtc2(net->nd[to].cpu, COP2_MSG, COP2_MSG_HOPS, hops);

	CPU_mtc2(net->nd[to].cpu, COP2_MSG, COP2_MSG_ST, COP2_MSG_OP_NONE);

	net->nd[to].mbox[from] = NULL;
	free(msg);

#ifdef VERBOSE
	warnx("%s input -- nd[%lu] cycle %lu -- %lu?data",
	      __FILE__, to, net->cycle, from);
#endif

	return 0;
}

/*
 * output: output command
 *
 * net:  network object
 * to:   target of the message
 * from: sender of the message
 *
 * Returns 0 if success, an error number otherwise.
 *
 * This function fails if:
 *	EDEADLK: deadlock.
 *	EAGAIN:  acknowledge not arrived yet, try later.
 *	ENOMEM:  could not allocate message.
 *	ENOBUFS: link buffer full.
 */
static int
output(Net *net, size_t to, size_t from)
{
	int             link;

	uint32_t        st;

	struct msg     *msg;

	struct link    *olink;

	st = CPU_mfc2(net->nd[from].cpu, COP2_MSG, COP2_MSG_ST);

	/*
	 * check if waiting for acknowledge
	 */
	if (COP2_MSG_ST_SENT(st)) {
		if ((msg = net->nd[from].mbox[to])) {
			if (msg->ack) {
				CPU_mtc2(net->nd[from].cpu, COP2_MSG,
					 COP2_MSG_ST, COP2_MSG_OP_NONE);
				net->nd[from].mbox[to] = NULL;
				free(msg);
				return 0;
			} else {
				warnx("%s output -- nd[%lu] cycle %lu -- "
				      "expected ack from %lu",
				      __FILE__, from, net->cycle, to);
				return EDEADLK;
			}
		} else {
			return EAGAIN;
		}
	}

	/*
	 * send message
	 */
	if (!(msg = malloc(sizeof(*msg)))) {
		warn("%s output -- nd[%lu] cycle %lu -- could not output",
		      __FILE__, from, net->cycle);
		return ENOMEM;
	}

	link = guidance(net, from, to);
	olink = &(net->nd[from].link[LINK_OUT][link]);
	if (olink->len >= LINK_BUFSZ) {
		warnx("%s output -- nd[%lu] cycle %lu -- "
		      "could not send message (link %s full)",
		      __FILE__, from, net->cycle, linkname[link]);
		return ENOBUFS;
	}

	msg->to = to;
	msg->from = from;
	msg->data = CPU_mfc2(net->nd[from].cpu, COP2_MSG, COP2_MSG_DATA);
	msg->ack = 0;
	msg->hops = 0;
	msg->nxt = NULL;

	link_insmsg(olink, msg);

	/* set sent flag */
	st |= 0x4;
	CPU_mtc2(net->nd[from].cpu, COP2_MSG, COP2_MSG_ST, st);

#ifdef VERBOSE
	warnx("%s output -- nd[%lu] cycle %lu -- %lu!data",
	      __FILE__, from, net->cycle, to);
#endif

	return 0;
}

/*
 * alt: alternative command
 *
 * net:     network object
 * clauses: array containg clauses
 * to:      target processor
 *
 * This function returns 0 if syccessfull, an error number otherwise.
 *
 * This function fails if:
 *	EDEADLK: deadlock.
 *	EAGAIN:  no message arrived yet, try later.
 *	ENOMEM:  could not allocate an object.
 *	ENOBUFS: link buffer full.
 */
static int
alt(Net *net, uint32_t *clauses, size_t to)
{
	int             link;

	int             done, found;

	uint32_t        from, hops, ncl;

	size_t          i, cur;

	struct msg     *msg, *ack;
	struct link    *olink;

	ncl = CPU_mfc2(net->nd[to].cpu, COP2_MSG, COP2_MSG_NCL);

	if (ncl <= 0) {
		warnx("%s alt -- nd[%lu] cycle %lu -- NCL <= 0",
		      __FILE__, to, net->cycle);
		return EDEADLK;
	}

	if (!net->nd[to].mbox_new) {
		return EAGAIN;
	}

	--net->nd[to].mbox_new;

	cur = net->nd[to].mbox_start;
	found = done = 0;
	while (!done) {
		if (!net->nd[to].mbox[cur]) {
			goto INC_CB;
		}
		for (i = 0; i < ncl; ++i) {
			if (clauses[i] == cur) {
				done = found = 1;
				from = cur;
#ifdef VERBOSE
				warnx("%s alt -- nd[%lu] cycle %lu -- "
				      "%u?data", __FILE__, to, net->cycle,
				      from);
#endif
				break;
			}
		}
INC_CB:
		cur = (cur + 1) % net->size;
		if (cur == net->nd[to].mbox_start) {
			done = 1;
		}
	}

	/* increment round-robin counter */
	net->nd[to].mbox_start = (net->nd[to].mbox_start + 1) % net->size;

	if (!found) {
		return EAGAIN;
	}

	msg = net->nd[to].mbox[from];

	if (!(ack = malloc(sizeof(*ack)))) {
		warn("%s alt -- nd[%lu] cycle %lu -- could not allocate ack",
		      __FILE__, to, net->cycle);
		return ENOMEM;
	}

	ack->to = msg->from;
	ack->from = to;
	ack->ack = 1;
	ack->hops = msg->hops;
	ack->nxt = NULL;

	link = guidance(net, to, ack->to);
	olink = &(net->nd[to].link[LINK_OUT][link]);
	if (olink->len >= LINK_BUFSZ) {
		warnx("%s alt -- nd[%lu] cycle %lu -- "
		      "could not send acknowledge (link %s full)",
		      __FILE__, to, net->cycle, linkname[link]);
		return ENOBUFS;
	}

	link_insmsg(olink, ack);

	CPU_mtc2(net->nd[to].cpu, COP2_MSG, COP2_MSG_DATA, msg->data);

	hops = CPU_mfc2(net->nd[to].cpu, COP2_MSG, COP2_MSG_HOPS);
	hops += msg->hops;
	CPU_mtc2(net->nd[to].cpu, COP2_MSG, COP2_MSG_HOPS, hops);

	CPU_mtc2(net->nd[to].cpu, COP2_MSG, COP2_MSG_ST, COP2_MSG_OP_NONE);

	net->nd[to].mbox[from] = NULL;
	free(msg);

	net->nd[to].cpu->gpr[K1] = from;

	return 0;
}

/*
 * guidance: get the direction of the next step to the target processor
 *
 * net:  network
 * from: current processor
 * to:   target processor
 *
 * Returns the direction of the next step.
 */
static
int
guidance(Net *net, size_t from, size_t to)
{
	if (from < to) {
		if ((from % net->y) < (to % net->y)) {
			return LINK_EAST;
		}
		return LINK_NORTH;
	}

	if (from > to) {
		if ((from % net->y) > (to % net->y)) {
			return LINK_WEST;
		}
		return LINK_SOUTH;
	}

	return LINK_MBOX;
}

/*
 * fwd:	foward messages from input links to output links or mailbox
 *
 * net: network
 * id:  id of current node
 */
static
void
fwd(Net *net, size_t id)
{
	int             ilink, olink;

	uint32_t        nmsg;

	struct msg     *msg;

	struct link    *in, *out;

	for (ilink = LINK_NORTH; ilink < LINK_NAMES; ++ilink) {
		in = &(net->nd[id].link[LINK_IN][ilink]);

		if (in->len == 0) {
			continue;
		}

		msg = in->hd;

		olink = guidance(net, id, msg->to);
		if (olink != LINK_MBOX) {
			out = &(net->nd[id].link[LINK_OUT][olink]);
			if (out->len == LINK_BUFSZ) {
				warnx("%s fwd -- nd[%lu] cycle %lu -- "
				      "output link %s is full",
				      __FILE__, id, net->cycle,
				      linkname[olink]);
				continue;
			}
		} else if (net->nd[id].mbox[msg->from]) {
			warnx("%s fwd -- nd[%lu] cycle %lu -- mailbox is full",
			      __FILE__, id, net->cycle);
			continue;
		}

		msg = link_remmsg(in);

		if (olink != LINK_MBOX) {
			link_insmsg(out, msg);
		} else {
			net->nd[id].mbox[msg->from] = msg;
			nmsg = CPU_mfc2(net->nd[id].cpu, COP2_MSG,
					COP2_MSG_NMSG);
			CPU_mtc2(net->nd[id].cpu, COP2_MSG,
				 COP2_MSG_NMSG, nmsg + 1);
			++net->nd[id].mbox_new;
		}
	}
}

/*
 * hop:	transmit messages from output links of one node to the input link of
 *	another node
 *
 * net: network
 * id:  id of the current node
 */
static void
hop(Net *net, uint32_t id)
{
	int             ilink, olink;
	int             oob;	/* out of bounds */

	uint32_t        nxt;

	struct msg     *msg;

	struct link    *out, *in;

	for (olink = LINK_NORTH; olink < LINK_NAMES; ++olink) {
		oob = 0;
		nxt = 0;
		ilink = olink;
		switch (olink) {
		case LINK_NORTH:
			nxt = id + net->x;
			ilink = LINK_SOUTH;
			oob = (nxt >= net->size) || (nxt < id);
			break;
		case LINK_EAST:
			nxt = id + 1;
			ilink = LINK_WEST;
			oob = (nxt % net->y) == 0;
			break;
		case LINK_SOUTH:
			nxt = id - net->x;
			ilink = LINK_NORTH;
			oob = nxt > id;
			break;
		case LINK_WEST:
			nxt = id - 1;
			ilink = LINK_EAST;
			oob = ((nxt % net->y) == (net->y - 1)) || (nxt > id);
			break;
		}
		out = &(net->nd[id].link[LINK_OUT][olink]);
		if (out->len == 0) {
			continue;
		}
		if (oob) {	/* out of bounds */
			warnx("%s hop -- nd[%u] cycle %lu -- "
			      "Can not send message out of the network (%s)",
			      __FILE__, id, net->cycle, linkname[olink]);
			continue;
		}
		in = &(net->nd[nxt].link[LINK_IN][ilink]);
		if (in->len == LINK_BUFSZ) {
			warnx("%s hop -- nd[%u] cycle %lu -- "
			      "target link %s is full",
			      __FILE__, id, net->cycle, linkname[ilink]);
			continue;
		}
		msg = link_remmsg(out);
		++(net->nd[id].linkutil[LINK_OUT][olink]);
		++msg->hops;
		link_insmsg(in, msg);
		++(net->nd[nxt].linkutil[LINK_IN][ilink]);
	}
}

/*
 * operate: execute an network related instruction
 *
 * net: network
 * id:  node id
 *
 * Returns 0 if success, an error number otherwise.
 *
 * This function fails if:
 *	EAGAIN:  try later.
 *	ENOMEM:  could not allocate an object.
 *	EDEADLK: deadlock detected.
 *	ENOBUFS: link buffer full.
 *	EINVAL:  operation does not exist.
 */
static
int
operate(Net *net, size_t id)
{
	uint32_t        corr, st;

	uint32_t       *clauses;

	corr = CPU_mfc2(net->nd[id].cpu, COP2_MSG, COP2_MSG_CORR);
	st = CPU_mfc2(net->nd[id].cpu, COP2_MSG, COP2_MSG_ST);

	switch (COP2_MSG_ST_OP(st)) {
	case COP2_MSG_OP_NONE:
		return 0;
	case COP2_MSG_OP_IN:
		return input(net, corr, id);	/* 0, EAGAIN, ENOMEM,
						 * ENOBUFS */
	case COP2_MSG_OP_OUT:
		return output(net, corr, id);	/* 0, EDEADLK, EAGAIN,
						 * ENOMEM, ENOBUFS */
	case COP2_MSG_OP_ALT:
		clauses = Mem_getptr(net->nd[id].mem, corr);
		return alt(net, clauses, id);	/* 0, EDEADLK, EAGAIN,
						 * ENOMEM, ENOBUFS */
	default:
		warnx("unreconized operation");
	}

	return EINVAL;
}

/*
 * execute: network cycle
 *
 * net: network
 *
 * Returns 0 if success, -1 otherwise.
 */
static
int
execute(Net *net)
{
	int             errnum;

	size_t          i;

	for (i = 0; i < net->size; ++i) {
		fwd(net, i);	 /* deal with old messages internally first */
	}

	for (i = 0; i < net->size; ++i) {
		errnum = operate(net, i); /* do operation */
		if (errnum == EDEADLK) {
			warnx("%s execute -- operate nd[%lu] cycle %lu -- "
			      "*** DEADLOCK ***", __FILE__, i, net->cycle);
			return -1;
		}
	}

	for (i = 0; i < net->size; ++i) {
		hop(net, i);	 /* send messages to next nodes */
	}

	return 0;
}

/*
 * Net_create: create cpu network
 *
 * x:     number of processors on the x axis
 * y:     number of processors on the y axis
 * memsz: memory size
 *
 * Returns network if success, NULL otherwise. In case of failure, errno is set
 * to correspondent error number.
 *
 *	ENOMEM: Could not allocate the network object because some memory
 *		allocation failed.
 */
Net *
Net_create(size_t x, size_t y, size_t memsz)
{
	size_t          i;

	Net            *net;

	errno = 0;

	if (!(net = malloc(sizeof(Net)))) {
		errno = ENOMEM;
		return NULL;
	}

	net->cycle = 0;

	net->x = x;
	net->y = y;
	net->size = x * y;
	net->nrun = net->size;

	if (!(net->nd = malloc(sizeof(struct node) * net->size))) {
		errno = ENOMEM;
		return NULL;
	}

	for (i = 0; i < net->size; ++i) {
		if (!(net->nd[i].cpu = CPU_create(i))) {
			errno = ENOMEM;
			return NULL;
		}

		if (!(net->nd[i].mem = Mem_create(memsz, 1))) {
			errno = ENOMEM;
			return NULL;
		}

		memset(net->nd[i].link, 0,
		       sizeof(struct link) * LINK_DIR * LINK_NAMES);

		memset(net->nd[i].linkutil, 0,
		       sizeof(size_t) * LINK_DIR * LINK_NAMES);

		net->nd[i].mbox_start = 0;
		net->nd[i].mbox_new = 0;
		if (!(net->nd[i].mbox =
		      malloc(sizeof(struct msg *) * net->size))) {
			errno = ENOMEM;
			return NULL;
		}
	}

	return net;
}

/*
 * Net_destroy: deallocate network object
 *
 * net: network object
 */
void
Net_destroy(Net *net)
{
	size_t          i;

	for (i = 0; i < net->size; ++i) {
		CPU_destroy(net->nd[i].cpu);
		Mem_destroy(net->nd[i].mem);
	}
}

/*
 * Net_setpc: set a program counter to a cpu on the network
 *
 * net: network
 * id: id of the cpu to set the program counter
 * pc: program counter
 *
 */
inline void
Net_setpc(Net *net, size_t id, uint32_t pc)
{
	CPU_setpc(net->nd[id].cpu, pc);
}

/*
 * Net_progld: loads program to all nodes in the network
 *
 * net: network
 * elf: ELF image
 *
 * Returns 0 if success, -1 otherwise. In case of failuere errno indicates the
 * error
 *
 * This function fails if:
 *	ENOSPC: could not load because the segment is out of bounds (inherited
 *		from Mem_progld).
 */
int
Net_progld(Net *net, size_t memsz, unsigned char *elf)
{
	size_t          i;

	errno = 0;

	if (Mem_progld(net->nd[0].mem, elf)) {
		return -1;
	}

	for (i = 1; i < net->size; ++i) {
		memcpy(net->nd[i].mem->data.b, net->nd[0].mem->data.b, memsz);
	}

	return 0;
}

/*
 * Net_runsim: run simulation
 *
 * net: network
 */
void
Net_runsim(Net *net)
{
	int             errcode;

	size_t          i;

	while (net->nrun > 0) {
		for (i = 0; i < net->size; ++i) {
			if (Datapath_execute(net->nd[i].cpu, net->nd[i].mem)) {
				continue;
			}
			if (!net->nd[i].cpu->running) {
				--net->nrun;
			}
		}
		errcode = execute(net);
		if (!errcode) {
			++net->cycle;
		} else {
			return;
		}
		fflush(stdout);
	}
}

/*
 * Net_perfct: print performance counters of the simulation
 *
 * net: network
 */
void
Net_perfct(Net *net, char *progname)
{
	int             fd;

	size_t          i, j, k;

	char            perfct[NAME_MAX];

	sprintf(perfct, "./perfct_%s.csv", basename(progname));
	if ((fd = open(perfct, O_WRONLY | O_CREAT | O_TRUNC,
		       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
		err(EXIT_FAILURE, "open: %s", perfct);

	dprintf(fd, "id,cycles,"
		"loads,ld defer,"
		"stores,st defer,"
		"ll,ll defer,"
		"sc,sc defer,"
		"rmail,"
		"ct0,"
		"ninput,noutput,"
		"commwait,"
		"hops,nmsg,"
		"linkutil[in].north,linkutil[in].east,"
		"linkutil[in].south,linkutil[in].west,"
		"linkutil[out].north,linkutil[out].east,"
		"linkutil[out].south,linkutil[out].west\n");
	dprintf(fd, "net,%lu\n", net->cycle);
	for (i = 0; i < net->size; ++i) {
		dprintf(fd, "%u,%lu,"	/* id,cycles */
			"%lu,%lu,"	/* loads,ld defer */
			"%lu,%lu,"	/* stores, st defer */
			"%lu,%lu,"	/* ll, ll defer */
			"%lu,%lu,%lu,"	/* sc, sc defer,rmwfail */
			"%lu,"		/* ct0 */
			"%lu,%lu,"	/* ninput, noutput */
			"%lu,"		/* commwait */
			"%lu,%lu",	/* hops, nmsg */
			net->nd[i].cpu->gpr[K0],
			net->nd[i].cpu->perfct.cycle,
			net->nd[i].cpu->perfct.ld,
			net->nd[i].cpu->perfct.lddefer,
			net->nd[i].cpu->perfct.st,
			net->nd[i].cpu->perfct.stdefer,
			net->nd[i].cpu->perfct.ll,
			net->nd[i].cpu->perfct.lldefer,
			net->nd[i].cpu->perfct.sc,
			net->nd[i].cpu->perfct.scdefer,
			net->nd[i].cpu->perfct.rmwfail,
			net->nd[i].cpu->perfct.ct[0].ct,
			net->nd[i].cpu->perfct.nin,
			net->nd[i].cpu->perfct.nout,
			net->nd[i].cpu->perfct.commwait,
			CPU_mfc2(net->nd[i].cpu, COP2_MSG, COP2_MSG_HOPS),
			CPU_mfc2(net->nd[i].cpu, COP2_MSG, COP2_MSG_NMSG));
		for (j = 0; j < LINK_DIR; ++j) {
			for (k = 0; k < LINK_NAMES; ++k) {
				dprintf(fd, ",%lu", net->nd[i].linkutil[j][k]);
			}
		}
		dprintf(fd, "\n");
	}

	if (close(fd) < 0) {
		err(EXIT_FAILURE, "close");
	}
}

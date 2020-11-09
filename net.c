/* Check LICENSE file for copyright and license details. */

/*
 * Network node
 */

#include <err.h>
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

#define X(a, b) b,
static const char *Net_errlist[] = {
	NetErrList
};
#undef X

#define Y(a, b) b,
static const char *linkname[] = {
	LinknameList
};
#undef Y

static int         link_insmsg(struct link *, struct msg *);
static struct msg *link_remmsg(struct link *);

static int      guidance(Net *, size_t, size_t);
static void     fwd(Net *, size_t);
static void     hop(Net *, uint32_t);
static void     operate(Net *, size_t);

Net            *Net_create(size_t, size_t, size_t);
void            Net_destroy(Net *);

void            Net_setpc(Net *, size_t, uint32_t);

int             Net_progld(Net *, size_t, unsigned char *);

void            Net_runsim(Net *);

const char     *Net_strerror(int);

/*
 * link_insmsg: insert message on link
 *
 * link: link to operate
 * msg: message to insert
 *
 * Returns 0 if successfully inserted on the link's circular buffer, -1
 * otherwise
 */
static
int
link_insmsg(struct link * link, struct msg * msg)
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
 * Returns a pointer to the removed message if sucessfull, NULL otherwise
 */
static
struct msg     *
link_remmsg(struct link * link)
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
 * guidance: get the direction of the next step to the target processor
 *
 * net: network
 * from: current processor
 * to: target processor
 *
 * Returns the direction of the next step
 */
static
int
guidance(Net *net, size_t from, size_t to)
{
	if ((from % net->x) < (to % net->x)) {
		return LINK_EAST;
	} else if ((from % net->x) > (to % net->x)) {
		return LINK_WEST;
	} else if (from < to) {
		return LINK_NORTH;
	} else if (from > to) {
		return LINK_SOUTH;
	}

	return LINK_MBOX;
}

/*
 * fwd:	foward messages from input links to output links or mailbox
 *
 * net: network
 * id: id of current node
 */
static
void
fwd(Net *net, size_t id)
{
	int             ilink, olink;

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
				warnx("net->nd[%lu] cycle %lu -- "
				      "output link %s is full",
				      id, net->cycle, linkname[olink]);
				continue;
			}
		} else if (net->nd[id].mbox[msg->from]) {
			warnx("net->nd[%lu] cycle %lu -- mailbox is full",
			      id, net->cycle);
			continue;
		}

		msg = link_remmsg(in);

		if (olink != LINK_MBOX) {
			link_insmsg(out, msg);
		} else {
			net->nd[id].mbox[msg->from] = msg;
		}
	}
}

/*
 * hop:	transmit messages from output links of one node to the input link of
 *	another node
 *
 * net: network
 * id: id of the current node
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
			oob = (nxt % net->x) == 0;
			break;
		case LINK_SOUTH:
			nxt = id - net->x;
			ilink = LINK_NORTH;
			oob = nxt > id;
			break;
		case LINK_WEST:
			nxt = id - 1;
			ilink = LINK_EAST;
			oob = ((nxt % net->x) == (net->x - 1)) || (nxt > id);
			break;
		}

		out = &(net->nd[id].link[LINK_OUT][olink]);
		if (out->len == 0) {
			continue;
		}

		if (oob) {	/* out of bounds */
			warnx("net->nd[%u] cycle %lu -- "
			      "Can not send message out of the network (%s)",
			      id, net->cycle, linkname[olink]);
			continue;
		}

		in = &(net->nd[nxt].link[LINK_IN][ilink]);
		if (in->len == LINK_BUFSZ) {
			warnx("net->nd[%u] cycle %lu -- "
			      "target link %s is full",
			      id, net->cycle, linkname[ilink]);
			continue;
		}

		msg = link_remmsg(out);
		++msg->hops;
		link_insmsg(in, msg);
	}
}

static
void
operate(Net *net, size_t id)
{
	int             link;

	uint32_t        corr, data, st,  hops, nmsg;

	struct msg     *msg, *ack;
	struct link    *olink;

	corr = CPU_mfc2(net->nd[id].cpu, COP2_MSG, COP2_MSG_CORR);
	data = CPU_mfc2(net->nd[id].cpu, COP2_MSG, COP2_MSG_DATA);
	st = CPU_mfc2(net->nd[id].cpu, COP2_MSG, COP2_MSG_ST);
	hops = CPU_mfc2(net->nd[id].cpu, COP2_MSG, COP2_MSG_HOPS);
	nmsg = CPU_mfc2(net->nd[id].cpu, COP2_MSG, COP2_MSG_NMSG);

	switch (COP2_MSG_ST_OP(st)) {
	case COP2_MSG_OP_IN:
		if ((msg = net->nd[id].mbox[corr])) {
#ifndef NDEBUG
			warnx("net->nd[%lu] cycle %lu -- i%u",
			      id, net->cycle, corr);
#endif
			if (!(ack = malloc(sizeof(*ack)))) {
				warnx("net->nd[%lu] cycle %lu --"
				      "could not allocate ack",
				      id, net->cycle);
				return;
			}
			++nmsg;
			hops += msg->hops;
			CPU_mtc2(net->nd[id].cpu, COP2_MSG,
				 COP2_MSG_DATA, msg->data);
			CPU_mtc2(net->nd[id].cpu, COP2_MSG,
				 COP2_MSG_NMSG, nmsg);
			CPU_mtc2(net->nd[id].cpu, COP2_MSG,
				 COP2_MSG_HOPS, hops);
			ack->to = msg->from;
			ack->from = id;
			ack->ack = 1;
			ack->hops = msg->hops;
			ack->nxt = NULL;
			link = guidance(net, id, ack->to);
			olink = &(net->nd[id].link[LINK_OUT][link]);
			if (olink->len < LINK_BUFSZ) {
				link_insmsg(olink, ack);
				CPU_mtc2(net->nd[id].cpu, COP2_MSG,
					 COP2_MSG_ST, 0);
				CPU_mtc2(net->nd[id].cpu, COP2_MSG,
					 COP2_MSG_NMSG, nmsg);
				net->nd[id].mbox[corr] = NULL;
				free(msg);
			} else {
				warnx("net->nd[%lu] cycle %lu -- "
				      "could not send "
				      "acknowledge (link %s full)",
				      id, net->cycle, linkname[link]);
			}
		}
		break;
	case COP2_MSG_OP_OUT:
		if (COP2_MSG_ST_SENT(st)) {	/* waiting for ack */
			if ((msg = net->nd[id].mbox[corr])) {
				if (msg->ack) {
					++nmsg;
					CPU_mtc2(net->nd[id].cpu, COP2_MSG,
						 COP2_MSG_ST, 0);
					CPU_mtc2(net->nd[id].cpu, COP2_MSG,
						 COP2_MSG_NMSG, nmsg);
					net->nd[id].mbox[corr] = NULL;
					free(msg);
				} else {
					warnx("net->nd[%lu] cycle %lu -- "
					      "*** DEADLOCK (%u) ***",
					      id, net->cycle, corr);
					Net_errno = NETERR_DEADLOCK;
				}
			}
		} else if ((msg = malloc(sizeof(*msg)))) {	/* send msg */
			link = guidance(net, id, corr);
			olink = &(net->nd[id].link[LINK_OUT][link]);
			if (olink->len < LINK_BUFSZ) {
				msg->to = corr;
				msg->from = id;
				msg->data = data;
				msg->ack = 0;
				msg->hops = 0;
				msg->nxt = NULL;
				link_insmsg(olink, msg);
				st |= 0x4;	/* set sent flag */
				CPU_mtc2(net->nd[id].cpu, COP2_MSG,
					 COP2_MSG_ST, st);
#ifndef NDEBUG
				warnx("net->nd[%lu] cycle %lu -- o%u",
				      id, net->cycle, corr);
#endif
			} else {
				warnx("net->nd[%lu] cycle %lu -- "
				      "could not send message "
				      "(link %s full)",
				      id, net->cycle, linkname[link]);
			}
		} else {
			warnx("net->nd[%lu] cycle %lu -- could not output",
			      id, net->cycle);
		}
		break;
	}
}

/*
 * Net_create: create cpu network
 *
 * x: number of processors on the x axis
 * y: number of processors on the y axis
 * memsz: memory size
 *
 * Returns network if success, NULL otherwise
 */
Net            *
Net_create(size_t x, size_t y, size_t memsz)
{
	size_t          i;

	Net            *net;

	Net_errno = NETERR_SUCC;

	if (!(net = malloc(sizeof(Net)))) {
		Net_errno = NETERR_ALLOC;
		return NULL;
	}
	net->cycle = 0;

	net->x = x;
	net->y = y;
	net->size = x * y;
	if (!(net->nd = malloc(sizeof(struct node) * net->size))) {
		Net_errno = NETERR_ALLOC;
		return NULL;
	}
	for (i = 0; i < net->size; ++i) {
		if (!(net->nd[i].cpu = CPU_create(i))) {
			Net_errno = NETERR_CPU;
			return NULL;
		}

		if (!(net->nd[i].mem = Mem_create(memsz, 1))) {
			Net_errno = NETERR_MEM;
			return NULL;
		}

		memset(net->nd[i].link, 0,
		       sizeof(struct link) * LINK_DIR * LINK_NAMES);

		if (!(net->nd[i].mbox =
		      malloc(sizeof(struct msg *) * net->size))) {
			Net_errno = NETERR_ALLOC;
			return NULL;
		}
	}

	return net;
}

/*
 * Net_destroy: deallocate network object
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
 * Returns 0 if success, -1 otherwise, Mem_errno indicates the error
 */
int
Net_progld(Net *net, size_t memsz, unsigned char *elf)
{
	size_t          i;

	if (Mem_progld(net->nd[0].mem, elf) < 0)
		return -1;

	for (i = 1; i < net->size; ++i)
		memcpy(net->nd[i].mem->data.b, net->nd[0].mem->data.b, memsz);

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
	size_t          i;

	for (i = 0; !Datapath_execute(net->nd[i].cpu, net->nd[i].mem); ++i) {
		if (i >= net->size) {
			++net->cycle;
			fflush(stdout);
			i = 0;
		}
	}

	if (Datapath_errno != DATAPATHERR_EXIT) {
		errx(EXIT_FAILURE, "net->nd[%lu] -- Datapath_execute %s",
		     i, Datapath_strerror(Datapath_errno));
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

	size_t          i;

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
		"rmwfail,"
		"ct0\n");
	dprintf(fd, "net,%lu\n", net->cycle);
	for (i = 0; i < net->size; ++i) {
		dprintf(fd, "%u,%lu,"	/* id,cycles */
			"%lu,%lu,"	/* loads,ld defer */
			"%lu,%lu,"	/* stores, st defer */
			"%lu,%lu,"	/* ll, ll defer */
			"%lu,%lu,%lu"	/* sc, sc defer,rmwfail */
			"%lu\n",/* ct0 */
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
			net->nd[i].cpu->perfct.ct[0].ct);
	}

	if (close(fd) < 0)
		err(EXIT_FAILURE, "close");
}
/*
 * Net_strerror: map error number to error message string
 *
 * code: error number
 *
 * Returns error message string
 */
inline const char *
Net_strerror(int code)
{
	return Net_errlist[code];
}

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

Net            *Net_create(size_t x, size_t y, size_t memsz);
void            Net_destroy(Net *net);

Net            *Net_create(size_t, size_t, size_t);
void            Net_destroy(Net *);

void            Net_setpc(Net *, size_t, uint32_t);

int             Net_progld(Net *, size_t, unsigned char *);

void            Net_runsim(Net *);

const char     *Net_strerror(int code);

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

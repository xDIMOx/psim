/* Check LICENSE file for copyright and license details. */

/*
 * Distributed memory implementation
 */

#include <sys/mman.h>

#include <elf.h>
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
#include "net.h"

#include "simutil.h"

/* Implements */
#include "dstbmem.h"

static Net     *create(size_t x, size_t y, size_t memsz);
static void     destroy(Net *);

static void     setpc(Net *, size_t, uint32_t);

static int      progld(Net *, size_t, unsigned char *);

static void     runsim(Net *);

static void     perfct(Net *, char *);

void            DstbMem_run(size_t, size_t, size_t, struct ProgInfo *);

/*
 * create: create cpu network
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
static Net *
create(size_t x, size_t y, size_t memsz)
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

	if (!(net->nd = malloc(sizeof(struct Node) * net->size))) {
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
		       sizeof(struct Link) * LINK_DIR * LINK_NAMES);

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
 * destroy: deallocate network object
 *
 * net: network object
 */
static void
destroy(Net *net)
{
	size_t          i;

	for (i = 0; i < net->size; ++i) {
		CPU_destroy(net->nd[i].cpu);
		Mem_destroy(net->nd[i].mem);
	}
}

/*
 * setpc: set a program counter to a cpu on the network
 *
 * net: network
 * id: id of the cpu to set the program counter
 * pc: program counter
 *
 */
static inline void
setpc(Net *net, size_t id, uint32_t pc)
{
	CPU_setpc(net->nd[id].cpu, pc);
}

/*
 * progld: loads program to all nodes in the network
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
static int
progld(Net *net, size_t memsz, unsigned char *elf)
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
 * runsim: run simulation
 *
 * net: network
 */
static void
runsim(Net *net)
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
		errcode = Net_execute(net);
		if (!errcode) {
			++net->cycle;
		} else {
			return;
		}
		fflush(stdout);
	}
}

/*
 * perfct: print performance counters of the simulation
 *
 * net: network
 */
static void
perfct(Net *net, char *progname)
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
		"rmwfail,"
		"ct0,"
		"ninput,noutput,nalt,"
		"waitin,waitout,waitalt,"
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
			"%lu,%lu,%lu,"	/* ninput, noutput, nalt */
			"%lu,%lu,%lu,"	/* waitin, waitout, waitalt */
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
			net->nd[i].cpu->perfct.nalt,
			net->nd[i].cpu->perfct.waitin,
			net->nd[i].cpu->perfct.waitout,
			net->nd[i].cpu->perfct.waitalt,
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

/*
 * sim_net: simulate a network of processors
 *
 * x: number of processors on the x axis
 * y: number of processors on the y axis
 * memsz: memory size
 * prog: information about the program to be executed
 */
void
DstbMem_run(size_t x, size_t y, size_t memsz, struct ProgInfo *prog)
{
	size_t          i;

	Net            *net;

	/*
	 * Create network
	 */
	if (!(net = create(x, y, memsz))) {
		err(EXIT_FAILURE, "create");
	}

	/*
	 * Program loading
	 */
	if (progld(net, memsz, prog->elf) < 0) {
		err(EXIT_FAILURE, "progld");
	}

	for (i = 0; i < (x * y); ++i) {
		setpc(net, i, prog->entry);
	}

	/* free memory after loading */
	if (munmap(prog->elf, prog->size) < 0) {
		err(EXIT_FAILURE, "munmap");
	}

	/*
	 * Run simulation
	 */
	runsim(net);
	if (errno) {
		err(EXIT_FAILURE, "runsim");
	}
	perfct(net, prog->name);
	destroy(net);
}

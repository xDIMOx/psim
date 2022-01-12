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

static CPU         *cpubank;

static Mem         *membank;

static struct Msg **mboxes;

static Net     *create(size_t, size_t, size_t, int32_t);
static void     destroy(Net *);

static int      progld(Net *, size_t, unsigned char *);

static int      sim(Net *);

static void     perfct(Net *, char *);

int             DstbMem_run(size_t, size_t, size_t, struct ProgInfo *);

/*
 * create: create cpu network
 *
 * x:     number of processors on the x axis
 * y:     number of processors on the y axis
 * memsz: memory size
 * entry: program entry point
 *
 * Returns network if success, NULL otherwise. In case of failure, errno is set
 * to correspondent error number.
 *
 * This function fails if:
 *	ENOMEM: Could not allocate the network object because some memory
 *		allocation failed.
 */
static Net *
create(size_t x, size_t y, size_t memsz, int32_t entry)
{
	size_t          i, off;

	Net            *net;

	errno = ENOMEM;		/* assume an error is going to happen */

	if (!(net = malloc(sizeof(Net)))) {
#ifdef VERBOSE
		warn("%s create -- malloc(net)", __FILE__);
#endif
		return NULL;
	}

	net->cycle = 0;

	net->x = x;
	net->y = y;
	net->size = x * y;
	net->nrun = net->size;

	/*
	 * allocate CPUs
	 */
	if (!(cpubank = CPU_create(net->size, entry))) {
#ifdef VERBOSE
		warn("%s create -- CPU_create(%lu,%u)", __FILE__,
		     net->size, entry);
#endif
		return NULL;
	}

	/*
	 * allocate memories
	 */
	if (!(membank = Mem_createarr(memsz, net->size))) {
#ifdef VERBOSE
		warn("%s create -- Mem_createarr", __FILE__);
#endif
		return NULL;
	}

	/*
	 * allocate mailboxes
	 */
	if (!(mboxes = malloc(sizeof(*mboxes) * net->size * net->size))) {
#ifdef VERBOSE
		warn("%s create -- malloc(mboxes)", __FILE__);
#endif
		return NULL;
	}

	/*
	 * allocate network
	 */
	if (!(net->nd = malloc(sizeof(struct Node) * net->size))) {
#ifdef VERBOSE
		warn("%s create -- malloc(net->nd)", __FILE__);
#endif
		return NULL;
	}
	memset(net->nd, 0, sizeof(struct Node) * net->size);

	for (off = i = 0; i < net->size; ++i, off += net->size) {
		net->nd[i].cpu  = &cpubank[i];
		net->nd[i].mem  = &membank[i];
		net->nd[i].mbox = &mboxes[off];
	}

	errno = 0;		/* no errors occured */

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
	CPU_destroy(cpubank);
	Mem_destroyarr(membank);
	free(mboxes);
	free(net);
}

/*
 * progld: loads program to all nodes in the network
 *
 * net: network
 * elf: ELF image
 *
 * Returns 0 if success, -1 otherwise.
 */
static int
progld(Net *net, size_t memsz, unsigned char *elf)
{
	size_t          i;

	errno = 0;

	if ((errno = Mem_progld(net->nd[0].mem, elf))) {
#ifdef VERBOSE
		warn("%s progld -- Mem_progld", __FILE__);
#endif
		return -1;
	}

	for (i = 1; i < net->size; ++i) {
		memcpy(net->nd[i].mem->data.b, net->nd[0].mem->data.b, memsz);
	}

	return 0;
}

/*
 * sim: run simulation
 *
 * net: network
 *
 * Returns 0 if success, -1 otherwise.
 */
static int
sim(Net *net)
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
			return -1;
		}
		fflush(stdout);
	}

	return 0;
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
 * DstbMem_run: simulate a network of processors
 *
 * x: number of processors on the x axis
 * y: number of processors on the y axis
 * memsz: memory size
 * prog: information about the program to be executed
 *
 * Returns 0 if the simulation completed successfully, -1 otherwise.
 */
int
DstbMem_run(size_t x, size_t y, size_t memsz, struct ProgInfo *prog)
{
	int             ret;

	Net            *net;

	/*
	 * Create network
	 */
	if (!(net = create(x, y, memsz, prog->entry))) {
		warn("DstbMem_run -- create");
		return -1;
	}

	/*
	 * Program loading
	 */
	if (progld(net, memsz, prog->elf) < 0) {
		warnx("DstbMem_run -- progld");
		return -1;
	}

	/* free memory after loading */
	if (munmap(prog->elf, prog->size) < 0) {
		warn("DstbMem_run -- munmap");
	}

	/*
	 * Run simulation
	 */
	ret = sim(net);
	if (ret) {
		warnx("DstbMem_run -- sim");
	}

	perfct(net, prog->name);

	destroy(net);

	return ret;
}

/* Check LICENSE file for copyright and license details. */

/*
 * Distributed memory implementation
 */

#include <sys/mman.h>

#include <elf.h>
#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include "cpu.h"
#include "mem.h"
#include "net.h"

#include "simutil.h"

/* Implements */
#include "dstbmem.h"

void            DstbMem_run(size_t, size_t, size_t, struct ProgInfo *);

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
	if (!(net = Net_create(x, y, memsz))) {
		err(EXIT_FAILURE, "Net_create");
	}

	/*
	 * Program loading
	 */
	if (Net_progld(net, memsz, prog->elf) < 0) {
		err(EXIT_FAILURE, "Net_progld");
	}

	for (i = 0; i < (x * y); ++i) {
		Net_setpc(net, i, prog->entry);
	}

	/* free memory after loading */
	if (munmap(prog->elf, prog->size) < 0) {
		err(EXIT_FAILURE, "munmap");
	}

	/*
	 * Run simulation
	 */
	Net_runsim(net);
	if (errno) {
		err(EXIT_FAILURE, "Net_runsim");
	}
	Net_perfct(net, prog->name);
	Net_destroy(net);
}

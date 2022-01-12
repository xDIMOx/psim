/* Check LICENSE file for copyright and license details. */

/*
 * Shared memory implementation
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
#include <unistd.h>

#include "simutil.h"

#include "cpu.h"
#include "mem.h"

#include "datapath.h"

/* Implements */
#include "shrmem.h"

static int      sim(CPU *, size_t, Mem *);

static void     perfct(CPU *, size_t, char *);

int             ShrMem_run(size_t, size_t, struct ProgInfo *);

/*
 * Program execution
 */
static int
sim(CPU *cpu, size_t ncpu, Mem *mem)
{
	size_t          i;
	size_t          nrun;

	nrun = ncpu;
	while (nrun > 0) {
		Mem_busclr();
		for (i = 0; i < ncpu; ++i) {
			if (Datapath_execute(&cpu[i], mem)) {
				continue;
			} else if (!cpu[i].running) {
				--nrun;
			}
		}
		fflush(stdout);
	}

	return 0;
}

static void
perfct(CPU *cpu, size_t ncpu, char *progname)
{
	int             fd;

	size_t          i;

	char            perfct[NAME_MAX];

	sprintf(perfct, "./perfct_%s.csv", basename(progname));
	if ((fd = open(perfct, O_WRONLY | O_CREAT | O_TRUNC,
		       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		warn("ShrMem_run -- open %s", perfct);
		return;
	}

	dprintf(fd, "bus,%lu\n", Mem_busutil());
	for (i = 0; i < ncpu; ++i) {
		dprintf(fd, "id,"
		            "cycles,"
		            "loads,ld defer,"
		            "stores,st defer,"
		            "ll,ll defer,"
		            "sc,sc defer,"
		            "rmwfail,"
		            "ct0\n"
		            "%u,"	/* id */
		            "%lu,"	/* cycles */
		            "%lu,%lu,"	/* loads,ld defer */
		            "%lu,%lu,"	/* stores, st defer */
		            "%lu,%lu,"	/* ll, ll defer */
		            "%lu,%lu,"	/* sc, sc defer */
		            "%lu,"	/* rmwfail */
		            "%lu\n",	/* ct0 */
			cpu[i].gpr[K0],
			cpu[i].perfct.cycle,
			cpu[i].perfct.ld, cpu[i].perfct.lddefer,
			cpu[i].perfct.st, cpu[i].perfct.stdefer,
			cpu[i].perfct.ll, cpu[i].perfct.lldefer,
			cpu[i].perfct.sc, cpu[i].perfct.scdefer,
			cpu[i].perfct.rmwfail,
			cpu[i].perfct.ct[0].ct);
	}

	if (close(fd) < 0) {
		warn("ShrMem_run -- close");
	}
}

/*
 * ShrMem_run: simulate a shared memory multiprocessor system
 *
 * ncpu: number of CPUs to be simulated
 * memsz: memory size
 * prog: information about the program to be executed
 *
 * Returns 0 if the simulation completed successfully, -1 otherwise.
 */
int
ShrMem_run(size_t ncpu, size_t memsz, struct ProgInfo *prog)
{
	int             ret;

	CPU            *cpu;

	Mem            *mem;

	ret = 0;

	if (!(cpu = CPU_create(ncpu, prog->entry))) {
		warn("ShrMem_run -- CPU_create");
		return -1;
	}

	if (!(mem = Mem_create(memsz, ncpu))) {
		warn("ShrMem_run -- Mem_create");
		return -1;
	}

	/*
	 * Program loading
	 */
	if ((errno = Mem_progld(mem, prog->elf))) {
		warn("ShrMem_run -- Mem_progld");
		return -1;
	}

	/* free memory after loading */
	if (munmap(prog->elf, prog->size) < 0) {
		warn("ShrMem_run -- munmap");
	}

	ret = sim(cpu, ncpu, mem);

	perfct(cpu, ncpu, prog->name);

	CPU_destroy(cpu);

	Mem_destroy(mem);

	return ret;
}

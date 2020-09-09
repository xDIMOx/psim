/* Check LICENSE file for copyright and license details. */

/*
 * MIPS32 simulator, it receives a little-endian MIPS32 binary as input,
 * load it, then tries to execute it.
 */

#include <sys/mman.h>
#include <sys/stat.h>

#include <elf.h>
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
#include "net.h"

#ifndef IS_ELF
#define IS_ELF(eh) ((eh).e_ident[EI_MAG0] == ELFMAG0 && \
                    (eh).e_ident[EI_MAG1] == ELFMAG1 && \
                    (eh).e_ident[EI_MAG2] == ELFMAG2 && \
                    (eh).e_ident[EI_MAG3] == ELFMAG3)
#endif

enum org {
	SHRMEM,
	NET,
};

struct prog {
	off_t           size;
	Elf32_Addr      entry;
	char           *name;
	unsigned char  *elf;
};

void            sim_shrmem(size_t ncpu, size_t memsz, struct prog * prog);
void            sim_net(size_t x, size_t y, size_t memsz, struct prog * prog);

int             main(int argc, char *argv[]);

/*
 * sim_shrmem: simulate a shared memory multiprocessor system
 *
 * ncpu: number of CPUs to be simulated
 * memsz: memory size
 * prog: information about the program to be executed
 */
void
sim_shrmem(size_t ncpu, size_t memsz, struct prog * prog)
{
	int             fd;

	size_t          i;

	CPU           **cpu;

	Mem            *mem;

	char            perfct[NAME_MAX];

	if (!(cpu = malloc(sizeof(CPU *) * ncpu)))
		err(EXIT_FAILURE, "could not allocate cpu array");

	for (i = 0; i < ncpu; ++i) {
		if (!(cpu[i] = CPU_create(i))) {
			errx(EXIT_FAILURE, "cpu[%lu] -- CPU_create: %s",
			     i, CPU_strerror(CPU_errno));
		}
	}

	if (!(mem = Mem_create(memsz, ncpu)))
		errx(EXIT_FAILURE, "Mem_create: %s", Mem_strerror(Mem_errno));

	/*
	 * Program loading
	 */
	if (Mem_progld(mem, prog->elf) < 0)
		errx(EXIT_FAILURE, "Mem_progld: %s", Mem_strerror(Mem_errno));

	for (i = 0; i < ncpu; ++i)
		CPU_setpc(cpu[i], prog->entry);

	/* free memory after loading */
	if (munmap(prog->elf, prog->size) < 0)
		err(EXIT_FAILURE, "munmap");

	/*
	 * Program execution
	 */
	for (i = 0; !Datapath_execute(cpu[i], mem); i = (i + 1) % ncpu) {
		if (i == (ncpu - 1)) {
			Mem_busclr();
			fflush(stdout);
		}
	}

	if (Datapath_errno != DATAPATHERR_EXIT) {
		errx(EXIT_FAILURE, "cpu[%lu] -- Datapath_execute %s",
		     i, Datapath_strerror(Datapath_errno));
	}
	Mem_destroy(mem);

	sprintf(perfct, "./perfct_%s.csv", basename(prog->name));
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
		"lockperf0_cycles,lockperf0_acc,"
		"lockperf1_cycles,lockperf1_acc,"
		"lockperf2_cycles,lockperf2_acc,"
		"lockperf3_cycles,lockperf3_acc,"
		"lockperf4_cycles,lockperf4_acc,"
		"lockperf5_cycles,lockperf5_acc\n");
	dprintf(fd, "bus,%lu\n", Mem_busutil());
	for (i = 0; i < ncpu; ++i) {
		dprintf(fd, "%u,"	/* id */
			"%lu,"	/* cycles */
			"%lu,%lu,"	/* loads,ld defer */
			"%lu,%lu,"	/* stores, st defer */
			"%lu,%lu,"	/* ll, ll defer */
			"%lu,%lu,"	/* sc, sc defer */
			"%lu,"	/* rmwfail */
			"%lu,"	/* ct0 */
			"%lu,%lu,"	/* lockperf0_cycles, lockperf0_acc */
			"%lu,%lu,"	/* lockperf1_cycles, lockperf1_acc */
			"%lu,%lu,"	/* lockperf2_cycles, lockperf2_acc */
			"%lu,%lu,"	/* lockperf3_cycles, lockperf3_acc */
			"%lu,%lu,"	/* lockperf4_cycles, lockperf4_acc */
			"%lu,%lu\n",	/* lockperf5_cycles, lockperf5_acc */
			cpu[i]->gpr[K0], cpu[i]->perfct.cycle,
			cpu[i]->perfct.ld, cpu[i]->perfct.lddefer,
			cpu[i]->perfct.st, cpu[i]->perfct.stdefer,
			cpu[i]->perfct.ll, cpu[i]->perfct.lldefer,
			cpu[i]->perfct.sc, cpu[i]->perfct.scdefer,
			cpu[i]->perfct.rmwfail,
			cpu[i]->perfct.ct[0].ct,
			cpu[i]->perfct.lockperf[0].cycle,
			cpu[i]->perfct.lockperf[0].acc,
			cpu[i]->perfct.lockperf[1].cycle,
			cpu[i]->perfct.lockperf[1].acc,
			cpu[i]->perfct.lockperf[2].cycle,
			cpu[i]->perfct.lockperf[2].acc,
			cpu[i]->perfct.lockperf[3].cycle,
			cpu[i]->perfct.lockperf[3].acc,
			cpu[i]->perfct.lockperf[4].cycle,
			cpu[i]->perfct.lockperf[4].acc,
			cpu[i]->perfct.lockperf[5].cycle,
			cpu[i]->perfct.lockperf[5].acc);
		CPU_destroy(cpu[i]);
	}

	if (close(fd) < 0)
		err(EXIT_FAILURE, "close");

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
sim_net(size_t x, size_t y, size_t memsz, struct prog * prog)
{
	size_t          i;

	Net            *net;

	/*
	 * Create network
	 */
	if (!(net = Net_create(x, y, memsz)))
		errx(EXIT_FAILURE, "Net_create: %s", Net_strerror(Net_errno));

	/*
	 * Program loading
	 */
	if (Net_progld(net, memsz, prog->elf) < 0)
		errx(EXIT_FAILURE, "Net_progld: %s", Mem_strerror(Mem_errno));

	for (i = 0; i < (x * y); ++i)
		Net_setpc(net, i, prog->entry);

	/* free memory after loading */
	if (munmap(prog->elf, prog->size) < 0)
		err(EXIT_FAILURE, "munmap");

	/*
	 * Run simulation
	 */
	Net_runsim(net);
	Net_perfct(net, prog->name);
	Net_destroy(net);
}

int
main(int argc, char *argv[])
{
	int             c;
	int             fd;
	int             flag;

	size_t          memsz;
	size_t          ncpu;
	size_t          x, y;

	Elf32_Ehdr     *eh;

	struct prog     prog;

	struct stat     stat;

	/*
	 * Default options
	 */
	memsz = (1 << 24);	/* 16 Mib */
	ncpu = 1;
	flag = SHRMEM;
	x = y = 1;

	/*
	 * Parse command line options
	 */
	while ((c = getopt(argc, argv, ":n:m:c:s")) != -1) {
		switch (c) {
		case 'c':
			ncpu = strtoul(optarg, NULL, 10);
			break;
		case 'n':
			flag = NET;
			x = strtoul(strtok(optarg, "x"), NULL, 10);
			y = strtoul(strtok(NULL, "x"), NULL, 10);
			break;
		case 'm':
			memsz = strtoul(optarg, NULL, 10);
			break;
		case 's':
			flag = SHRMEM;
			break;
		case ':':
			errx(EXIT_FAILURE, "Option -%c requires an operand",
			     optopt);
			break;
		case '?':
			errx(EXIT_FAILURE, "Unrecognized option: '-%c'",
			     optopt);
			break;
		}
	}

	prog.name = argv[optind];

	/*
	 * Read file
	 */
	if (!prog.name)
		errx(EXIT_FAILURE, "usage: %s /path/to/objfile", argv[0]);

	if ((fd = open(prog.name, O_RDONLY)) < 0)
		err(EXIT_FAILURE, "open: %s", prog.name);

	if (fstat(fd, &stat) < 0)
		err(EXIT_FAILURE, "fstat");

	prog.size = stat.st_size;
	if ((prog.elf = mmap(NULL, stat.st_size, PROT_READ,
			     MAP_PRIVATE, fd, 0)) == MAP_FAILED)
		err(EXIT_FAILURE, "mmap");

	/* Checks file type */
	eh = (Elf32_Ehdr *) prog.elf;
	if (!IS_ELF(*eh) || eh->e_ident[EI_CLASS] != ELFCLASS32 ||
	    eh->e_ident[EI_DATA] != ELFDATA2LSB || eh->e_type != ET_EXEC ||
	    eh->e_machine != EM_MIPS)
		errx(EXIT_FAILURE, "%s is not a MIPS32 EL executable",
		     prog.name);
	prog.entry = eh->e_entry;

	if (flag == SHRMEM)
		sim_shrmem(ncpu, memsz, &prog);
	else
		sim_net(x, y, memsz, &prog);

	if (close(fd) < 0)
		err(EXIT_FAILURE, "close");

	return EXIT_SUCCESS;
}

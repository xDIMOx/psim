/* Check LICENSE file for copyright and license details. */

/*
 * Processor's datapath
 */

#include <err.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifndef NDEBUG
#include <unistd.h>
#endif

#include "cpu.h"
#include "mem.h"
#include "instr.h"

/* Implements */
#include "datapath.h"

#define X(a, b) b,
static const char *Datapath_errlist[] = {
	DatapathErrList
};
#undef X

#define IO_ADDR 0xFFFC

static int64_t  fetch(CPU *cpu, Mem *mem);
static int      decode(Decoder *dec);
static int      execute(CPU *cpu, Mem *mem);

int             Datapath_execute(CPU *cpu, Mem *mem);

const char     *Datapath_strerror(int errno);

/*
 * fetch: Fetch an instruction
 *
 * cpu: CPU fetching the instruction
 * mem: memory where the instruction is
 *
 * Returns instruction if success, -1 otherwise
 */
static int64_t
fetch(CPU *cpu, Mem *mem)
{
	int64_t         instr;

	if ((instr = Mem_lw(mem, cpu->pc)) < 0) {
		warnx("cpu[%u] -- fetch Mem_lw: %s",
		      cpu->gpr[K0], Mem_strerror(Mem_errno));
		Datapath_errno = DATAPATHERR_FET;
		return -1;
	}
	return instr;
}

/*
 * decode: decode an instruction
 *
 * dec: struct containing decoding information
 *
 * Returns 0 if success, -1 otherwise
 */
static int
decode(Decoder *dec)
{
	dec->rd = RD(dec->raw) >> 11;
	dec->rs = RS(dec->raw) >> 21;
	dec->rt = RT(dec->raw) >> 16;
	dec->sa = SA(dec->raw) >> 6;

	dec->stall = 0;

	dec->imm = IMM(dec->raw);

	dec->isjump = 0;
	dec->idx = INSTR_IDX(dec->raw) << 2;

	dec->sign = OPC(dec->raw);
	switch (OPC(dec->raw) >> 26) {
	case SPECIAL:
		dec->sign |= FUNC(dec->raw);
		switch (FUNC(dec->raw)) {
		case MOVCI:
			dec->sign |= TF(dec->raw);
			break;
		case SRL_FIELD:
			dec->sign |= SHROT(dec->raw);
			break;
		case SRLV_FIELD:
			dec->sign |= SHROTV(dec->raw);
			break;
		case JR:
		case JALR:
			dec->isjump = 1;
			break;
		}
		return 0;
	case REGIMM:
		dec->sign |= RT(dec->raw);
		switch (dec->rt) {
		case BLTZ:
		case BGEZ:
		case BLTZL:
		case BGEZL:
		case TGEI:
		case TGEIU:
		case TLTI:
		case TLTIU:
		case TEQI:
		case TNEI:
		case BLTZAL:
		case BGEZAL:
		case BLTZALL:
		case BGEZALL:
			dec->isjump = 1;
			break;
		case SYNCI:
			break;
		}
		return 0;
	case J:
	case JAL:
	case BEQ:
	case BNE:
	case BLEZ:
	case BGTZ:
		dec->isjump = 1;
		return 0;
	case ADDI:
	case ADDIU:
	case SLTI:
	case SLTIU:
	case ANDI:
	case ORI:
	case XORI:
	case LUI:
		return 0;
	case COP0:
		if (dec->rs > COP0RS_IGN10)
			dec->sign |= FUNC(dec->raw);
		else
			dec->sign |= RS(dec->raw);
		return 0;
	case COP1:
		return 0;
	case COP2:
		if (dec->rs > COP2RS_IGN8)
			dec->sign |= CO(dec->raw);
		else
			dec->sign |= RS(dec->raw);
		return 0;
	case COP1X:
		return 0;
	case BEQL:
	case BNEL:
	case BLEZL:
	case BGTZL:
		dec->isjump = 1;
		return 0;
	case SPECIAL2:
		dec->sign |= FUNC(dec->raw);
		return 0;
	case JALX:
		dec->isjump = 1;
		return 0;
	case SPECIAL3:
		dec->sign |= FUNC(dec->raw);
		if (FUNC(dec->raw) == BSHFL)
			dec->sign |= SA(dec->raw);
		return 0;
	case LB:
	case LH:
	case LWL:
	case LW:
	case LBU:
	case LHU:
	case LWR:
	case SB:
	case SH:
	case SWL:
	case SW:
	case SWR:
	case CACHE:
	case LL:
	case LWC1:
	case LWC2:
	case PREF:
	case LDC1:
	case LDC2:
	case SC:
	case SWC1:
	case SWC2:
	case SDC1:
	case SDC2:
		dec->ismem = 1;
		return 0;
	default:
		Datapath_errno = DATAPATHERR_RES;
		return -1;
	}

	Datapath_errno = DATAPATHERR_DEC;

	return -1;
}

/*
 * execute: execute instruction
 *
 * cpu: CPU executing the instruction
 * mem: memory
 *
 * Returns 0 if success, -1 otherwise
 */
static int
execute(CPU *cpu, Mem *mem)
{
	int32_t         ext;
	uint32_t        off;
	uint32_t        addr;

	int64_t         data;

	Datapath_errno = DATAPATHERR_SUCC;

	addr = cpu->gpr[cpu->dec.rs] + cpu->dec.imm;

	switch (cpu->dec.sign) {
	case ((uint32_t) SPECIAL << 26) | SLL:
		cpu->gpr[cpu->dec.rd] = cpu->gpr[cpu->dec.rt] << cpu->dec.sa;
		break;
	case ((uint32_t) SPECIAL << 26) | JR:
		cpu->dec.npc = cpu->gpr[cpu->dec.rs];
		break;
	case ((uint32_t) SPECIAL << 26) | MOVZ:
		if (!cpu->gpr[cpu->dec.rt])
			cpu->gpr[cpu->dec.rd] = cpu->gpr[cpu->dec.rs];
		break;
	case ((uint32_t) SPECIAL << 26) | MOVN:
		if (cpu->gpr[cpu->dec.rt] != 0)
			cpu->gpr[cpu->dec.rd] = cpu->gpr[cpu->dec.rs];
		break;
	case ((uint32_t) SPECIAL << 26) | MFHI:
		cpu->gpr[cpu->dec.rd] = cpu->hilo.u32[HI];
		break;
	case ((uint32_t) SPECIAL << 26) | ADDU:
		cpu->gpr[cpu->dec.rd] = cpu->gpr[cpu->dec.rs] +
		    cpu->gpr[cpu->dec.rt];
		break;
	case ((uint32_t) SPECIAL << 26) | SUBU:
		cpu->gpr[cpu->dec.rd] = cpu->gpr[cpu->dec.rs] -
		    cpu->gpr[cpu->dec.rt];
		break;
	case ((uint32_t) SPECIAL << 26) | AND:
		cpu->gpr[cpu->dec.rd] = cpu->gpr[cpu->dec.rs] &
		    cpu->gpr[cpu->dec.rt];
		break;
	case ((uint32_t) SPECIAL << 26) | OR:
		cpu->gpr[cpu->dec.rd] = cpu->gpr[cpu->dec.rs] |
		    cpu->gpr[cpu->dec.rt];
		break;
	case ((uint32_t) REGIMM << 26) | (BLTZ << 16):
		if (((int32_t) cpu->gpr[cpu->dec.rs]) < 0) {
			ext = cpu->dec.imm;
			off = ext << 2;
			cpu->dec.npc = cpu->pc + 4 + off;
		} else
			cpu->dec.isjump = 0;
		break;
	case ((uint32_t) J << 26):
		cpu->dec.npc = (cpu->pc & 0xF0000000) | cpu->dec.idx;
		break;
	case ((uint32_t) JAL << 26):
		cpu->gpr[31] = cpu->pc + 8;
		cpu->dec.npc = (cpu->pc & 0xF0000000) | cpu->dec.idx;
		break;
	case ((uint32_t) BEQ << 26):
		if (cpu->gpr[cpu->dec.rs] == cpu->gpr[cpu->dec.rt]) {
			ext = cpu->dec.imm;
			off = ext << 2;
			cpu->dec.npc = cpu->pc + 4 + off;
		} else
			cpu->dec.isjump = 0;
		break;
	case ((uint32_t) BNE << 26):
		if (cpu->gpr[cpu->dec.rs] != cpu->gpr[cpu->dec.rt]) {
			ext = cpu->dec.imm;
			off = ext << 2;
			cpu->dec.npc = cpu->pc + 4 + off;
		} else
			cpu->dec.isjump = 0;
		break;
	case ((uint32_t) BLEZ << 26):
		if (((int32_t) cpu->gpr[cpu->dec.rs]) <= 0) {
			ext = cpu->dec.imm;
			off = ext << 2;
			cpu->dec.npc = cpu->pc + 4 + off;
		} else
			cpu->dec.isjump = 0;
		break;
	case ((uint32_t) BGTZ << 26):
		if (((int32_t) cpu->gpr[cpu->dec.rs]) > 0) {
			ext = cpu->dec.imm;
			off = ext << 2;
			cpu->dec.npc = cpu->pc + 4 + off;
		} else
			cpu->dec.isjump = 0;
		break;
	case ((uint32_t) ADDIU << 26):
		cpu->gpr[cpu->dec.rt] = cpu->gpr[cpu->dec.rs] + cpu->dec.imm;
		break;
	case ((uint32_t) SLTI << 26):
		cpu->gpr[cpu->dec.rt] = ((int32_t) cpu->gpr[cpu->dec.rs]) <
		    cpu->dec.imm;
		break;
	case ((uint32_t) SLTIU << 26):
		cpu->gpr[cpu->dec.rt] = (cpu->gpr[cpu->dec.rs]) <
		    (uint32_t) cpu->dec.imm;
		break;
	case ((uint32_t) ANDI << 26):
		cpu->gpr[cpu->dec.rt] = cpu->gpr[cpu->dec.rs] & cpu->dec.imm;
		break;
	case ((uint32_t) ORI << 26):
		cpu->gpr[cpu->dec.rt] = cpu->gpr[cpu->dec.rs] |
		    (uint16_t) cpu->dec.imm;
		break;
	case ((uint32_t) LUI << 26):
		cpu->gpr[cpu->dec.rt] = cpu->dec.imm << 16;
		break;
	case ((uint32_t) COP0 << 26) | WAIT:
		Datapath_errno = DATAPATHERR_EXIT;
		return -1;
	case ((uint32_t) COP2 << 26) | (1 << 25): /* coprocessor operation */
		switch (COFUN(cpu->dec.raw)) {
		case MEMSZ:
			cpu->gpr[K1] = mem->size;
			break;
		case CT0:
				cpu->perfct.ct[0].en = !cpu->perfct.ct[0].en;
				break;
		case LOCKPERF0:
				if (!cpu->perfct.lockperf[0].en)
					++cpu->perfct.lockperf[0].acc;
				cpu->perfct.lockperf[0].en =
				    !cpu->perfct.lockperf[0].en;
				break;
		case LOCKPERF1:
				if (!cpu->perfct.lockperf[1].en)
					++cpu->perfct.lockperf[1].acc;
				cpu->perfct.lockperf[1].en =
				    !cpu->perfct.lockperf[1].en;
				break;
		case LOCKPERF2:
				if (!cpu->perfct.lockperf[2].en)
					++cpu->perfct.lockperf[2].acc;
				cpu->perfct.lockperf[2].en =
				    !cpu->perfct.lockperf[2].en;
				break;
		case LOCKPERF3:
				if (!cpu->perfct.lockperf[3].en)
					++cpu->perfct.lockperf[3].acc;
				cpu->perfct.lockperf[3].en =
				    !cpu->perfct.lockperf[3].en;
				break;
		case LOCKPERF4:
				if (!cpu->perfct.lockperf[4].en)
					++cpu->perfct.lockperf[4].acc;
				cpu->perfct.lockperf[4].en =
				    !cpu->perfct.lockperf[4].en;
				break;
		case LOCKPERF5:
				if (!cpu->perfct.lockperf[5].en)
					++cpu->perfct.lockperf[5].acc;
				cpu->perfct.lockperf[5].en =
				    !cpu->perfct.lockperf[5].en;
				break;
		default:
			Datapath_errno = DATAPATHERR_IMPL;
			return -1;
		}
		break;
	case ((uint32_t) SPECIAL2 << 26) | MUL:
		cpu->gpr[cpu->dec.rd] = cpu->gpr[cpu->dec.rs] *
		    cpu->gpr[cpu->dec.rt];
		break;
	case ((uint32_t) SPECIAL3 << 26) | EXT:
		cpu->gpr[cpu->dec.rt] =
		    (cpu->gpr[cpu->dec.rs] >> LSB(cpu->dec.raw)) &
		    (0xFFFFFFFF >> (31 - MSBD(cpu->dec.raw)));
		break;
	case ((uint32_t) LB << 26):
		++cpu->perfct.ld;
		if (addr == IO_ADDR)
			cpu->gpr[cpu->dec.rt] = (int32_t) getchar();
		else {
			if (Mem_busacc(cpu->gpr[K0])) {
				cpu->dec.stall = 1;
				++cpu->perfct.lddefer;
#ifndef NDEBUG
				warnx("cpu[%u] -- Mem_lb: deferred",
				      cpu->gpr[K0]);
#endif
				return -1;
			}
			if ((data = Mem_lb(mem, addr)) < 0) {
				warnx("cpu[%u] -- Mem_lb: %s",
				      cpu->gpr[K0], Mem_strerror(Mem_errno));
				return -1;
			}
			/* sign extention */
			ext = (int8_t) data;
			cpu->gpr[cpu->dec.rt] = ext;
		}
		break;
	case ((uint32_t) LW << 26):
		++cpu->perfct.ld;
		if (Mem_busacc(cpu->gpr[K0])) {
			cpu->dec.stall = 1;
			++cpu->perfct.lddefer;
#ifndef NDEBUG
			warnx("cpu[%u] -- Mem_lw: deferred",
			      cpu->gpr[K0]);
#endif
			return -1;
		}
		if ((data = Mem_lw(mem, addr)) < 0) {
			warnx("cpu[%u] -- Mem_lw: %s",
			      cpu->gpr[K0], Mem_strerror(Mem_errno));
			return -1;
		}
		cpu->gpr[cpu->dec.rt] = data;
		break;
	case ((uint32_t) SB << 26):
		++cpu->perfct.st;
		if (addr == IO_ADDR)
			putchar((int) cpu->gpr[cpu->dec.rt]);
		else if (Mem_busacc(cpu->gpr[K0])) {
			cpu->dec.stall = 1;
			++cpu->perfct.stdefer;
#ifndef NDEBUG
			warnx("cpu[%u] -- Mem_sb: deferred",
			      cpu->gpr[K0]);
#endif
			return -1;
		} else if (Mem_sb(mem, addr, cpu->gpr[cpu->dec.rt])) {
			warnx("cpu[%u] -- Mem_sb: %s",
			      cpu->gpr[K0], Mem_strerror(Mem_errno));
			return -1;
		}
		break;
	case ((uint32_t) SW << 26):
		++cpu->perfct.st;
		if (addr == IO_ADDR)
			printf("%08x\n", cpu->gpr[cpu->dec.rt]);
		else if (Mem_busacc(cpu->gpr[K0])) {
			cpu->dec.stall = 1;
			++cpu->perfct.stdefer;
#ifndef NDEBUG
			warnx("cpu[%u] -- Mem_sw: deferred",
			      cpu->gpr[K0]);
#endif
			return -1;
		} else if (Mem_sw(mem, addr, cpu->gpr[cpu->dec.rt])) {
			warnx("cpu[%u] -- Mem_sw: %s",
			      cpu->gpr[K0], Mem_strerror(Mem_errno));
			return -1;
		}
		break;
	case ((uint32_t) LL << 26):
		++cpu->perfct.ll;
		if (Mem_busacc(cpu->gpr[K0])) {
			cpu->dec.stall = 1;
			++cpu->perfct.lldefer;
#ifndef NDEBUG
			warnx("cpu[%u] -- Mem_ll: deferred",
			      cpu->gpr[K0]);
#endif
			return -1;
		}
		if ((data = Mem_ll(mem, cpu->gpr[K0], addr)) < 0) {
			warnx("cpu[%u] -- Mem_ll: %s",
			      cpu->gpr[K0], Mem_strerror(Mem_errno));
			return -1;
		}
		cpu->gpr[cpu->dec.rt] = data;
		break;
	case ((uint32_t) SC << 26):
		++cpu->perfct.sc;
		if (Mem_busacc(cpu->gpr[K0])) {
			cpu->dec.stall = 1;
			++cpu->perfct.scdefer;
#ifndef NDEBUG
			warnx("cpu[%u] -- Mem_sc: deferred",
			      cpu->gpr[K0]);
#endif
			return -1;
		}
		Mem_sc(mem, cpu->gpr[K0], addr, cpu->gpr[cpu->dec.rt]);
		if (Mem_errno == MEMERR_SC) {	/* SC failed */
			cpu->gpr[cpu->dec.rt] = 0;
			++cpu->perfct.rmwfail;
		} else if (Mem_errno != MEMERR_SUCC) {	/* other errors */
			warnx("cpu[%u] -- Mem_sc: %s",
			      cpu->gpr[K0], Mem_strerror(Mem_errno));
			return -1;
		} else		/* SC successful */
			cpu->gpr[cpu->dec.rt] = 1;
		break;
	default:
		Datapath_errno = DATAPATHERR_IMPL;
		return -1;
	}

	return 0;
}

/*
 * Datapath_execute: execute a cycle
 *
 * cpu: CPU running at the moment
 * mem: memory that the CPU is using
 *
 * Returns 0 if success, -1 otherwise
 */
int
Datapath_execute(CPU *cpu, Mem *mem)
{
	size_t          i;

	int64_t         instr;

	Datapath_errno = DATAPATHERR_SUCC;

	cpu->gpr[0] = 0;	/* forces that r0 is zero */

	if ((instr = fetch(cpu, mem)) < 0)
		return -1;

#ifndef NDEBUG
	write(cpu->debug.fd, &instr, sizeof(uint32_t));
#endif

	cpu->dec.raw = (uint32_t) instr;
	if (decode(&cpu->dec))
		return -1;

	if (execute(cpu, mem) && !cpu->dec.stall)
		return -1;

	if (cpu->dec.isjump)
		cpu->pc = cpu->dec.npc;
	else if (!cpu->dec.stall)
		cpu->pc += 4;

	++cpu->perfct.cycle;
	for (i = 0; i < NLOCKPERF; ++i) {
		if (cpu->perfct.lockperf[i].en)
			++cpu->perfct.lockperf[i].cycle;
	}
	for (i = 0; i < NCOUNTERS; ++i) {
		if (cpu->perfct.ct[i].en)
			++cpu->perfct.ct[i].ct;
	}

	return 0;
}

/*
 * Datapath_strerror: Map error number to error message string
 *
 * errno: error number
 *
 * Returns error message string
 */
inline const char *
Datapath_strerror(int errno)
{
	return Datapath_errlist[errno];
}

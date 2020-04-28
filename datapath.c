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

static int64_t  fetch(CPU *cpu, Mem *mem);
static int      decode(Decoder *dec);

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
		warnx("fetch Mem_lw: %s", Mem_strerror(Mem_errno));
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
		dec->sign |= RS(dec->raw);
		if (dec->rs > COP0RS_IGN10)
			dec->sign |= FUNC(dec->raw);
		return 0;
	case COP1:
		return 0;
	case COP2:
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

int
execute(CPU *cpu, Mem *mem)
{
	int32_t         ext;
	uint32_t        off;
	uint32_t        addr;

	int64_t         data;

	addr = cpu->gpr[cpu->dec.rs] + cpu->dec.imm;

	switch (cpu->dec.sign) {
	case ((uint32_t) SPECIAL << 26) | SLL:
		cpu->gpr[cpu->dec.rd] = cpu->gpr[cpu->dec.rt] << cpu->dec.sa;
		break;
	case ((uint32_t) SPECIAL << 26) | JR:
		cpu->dec.npc = cpu->gpr[cpu->dec.rs];
		break;
	case ((uint32_t) SPECIAL << 26) | OR:
		cpu->gpr[cpu->dec.rd] = cpu->gpr[cpu->dec.rs] |
		    cpu->gpr[cpu->dec.rt];
		break;
	case ((uint32_t) J << 26):
		cpu->dec.npc = (cpu->pc & 0xF0000000) | cpu->dec.idx;
		break;
	case ((uint32_t) JAL << 26):
		cpu->gpr[31] = cpu->pc + 8;
		cpu->dec.npc = (cpu->pc & 0xF0000000) | cpu->dec.idx;
		break;
	case ((uint32_t) BNE << 26):
		if (cpu->gpr[cpu->dec.rs] != cpu->gpr[cpu->dec.rt]) {
			ext = cpu->dec.imm;
			off = ext << 2;
			cpu->dec.npc = cpu->pc + 4 + off;
		} else
			cpu->dec.isjump = 0;
		break;
	case ((uint32_t) ADDIU << 26):
		cpu->gpr[cpu->dec.rt] = cpu->gpr[cpu->dec.rs] + cpu->dec.imm;
		break;
	case ((uint32_t) ORI << 26):
		cpu->gpr[cpu->dec.rt] = cpu->gpr[cpu->dec.rs] | cpu->dec.imm;
		break;
	case ((uint32_t) LUI << 26):
		cpu->gpr[cpu->dec.rt] = cpu->dec.imm << 16;
		break;
	case ((uint32_t) LB << 26):
		if ((data = Mem_lb(mem, addr)) < 0) {
			warnx("Mem_lb: %s", Mem_strerror(Mem_errno));
			return -1;
		}
		/* sign extention */
		ext = (int8_t) data;
		cpu->gpr[cpu->dec.rt] = ext;
		break;
	case ((uint32_t) SB << 26):
		if (Mem_sb(mem, addr, (uint8_t) cpu->gpr[cpu->dec.rt])) {
			warnx("Mem_sb: %s", Mem_strerror(Mem_errno));
			return -1;
		}
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

	if (execute(cpu, mem))
		return -1;

	if (cpu->dec.isjump)
		cpu->pc = cpu->dec.npc;
	else
		cpu->pc += 4;

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

/* Check LICENSE file for copyright and license details. */

/*
 * Processor's datapath
 */

#include <err.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef INSTRDUMP
#include <unistd.h>
#endif

#include "cpu.h"
#include "mem.h"
#include "instr.h"

/* Implements */
#include "datapath.h"

#define IO_ADDR 0xFFFC

static int64_t  fetch(CPU *, Mem *);
static int      decode(Decoder *);
static int      execute(CPU *, Mem *);

int             Datapath_execute(CPU *, Mem *);

/*
 * fetch: Fetch an instruction
 *
 * cpu: CPU fetching the instruction
 * mem: memory where the instruction is
 *
 * Returns instruction if success, -1 otherwise.
 */
static int64_t
fetch(CPU *cpu, Mem *mem)
{
	int64_t         instr;

	if ((instr = Mem_lw(mem, cpu->pc)) < 0) {
		warn("%s fetch -- cpu[%u] cycle %lu -- Mem_lw",
		     __FILE__, cpu->gpr[K0], cpu->perfct.cycle);
		return -1;
	}

	return instr;
}

/*
 * decode: decode an instruction
 *
 * dec: struct containing decoding information
 *
 * Returns 0 if success, -1 otherwise.
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

	dec->sel = SEL(dec->raw);

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
		if (dec->rs > COP2RS_IGN8) {
			dec->sign |= CO(dec->raw);
		} else {
			dec->sign |= RS(dec->raw);
		}
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
		if (FUNC(dec->raw) == BSHFL) {
			dec->sign |= SA(dec->raw);
		}
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
	}

	return -1;
}

/*
 * execute: execute instruction
 *
 * cpu: CPU executing the instruction
 * mem: memory
 *
 * Returns 0 if success, an error number otherwise.
 *
 * This function fails if:
 *	EINVAL: invalid arguments
 *	ENOTSUP: instruction not implemented.
 *	EBUSY: resource not available now.
 *	EADDRNOTAVAIL: address is out of bounds.
 *	EFAULT: address is not word aligned.
 *	EOVERFLOW: tried to access reserved addresses array out of bounds.
 *	EAGAIN: SC failed.
 */
static int
execute(CPU *cpu, Mem *mem)
{
	int             errnum;

	int32_t         ext;
	uint32_t        off;
	uint32_t        addr;

	int64_t         data;

	addr = cpu->gpr[cpu->dec.rs] + cpu->dec.imm;

	switch (cpu->dec.sign) {
	case ((uint32_t) SPECIAL << 26) | SLL:
		cpu->gpr[cpu->dec.rd] = cpu->gpr[cpu->dec.rt] << cpu->dec.sa;
		break;
	case ((uint32_t) SPECIAL << 26) | SRA:
		cpu->gpr[cpu->dec.rd] = cpu->gpr[cpu->dec.rt] >> cpu->dec.sa;
		break;
	case ((uint32_t) SPECIAL << 26) | JR:
		cpu->dec.npc = cpu->gpr[cpu->dec.rs];
		break;
	case ((uint32_t) SPECIAL << 26) | MOVZ:
		if (!cpu->gpr[cpu->dec.rt]) {
			cpu->gpr[cpu->dec.rd] = cpu->gpr[cpu->dec.rs];
		}
		break;
	case ((uint32_t) SPECIAL << 26) | MOVN:
		if (cpu->gpr[cpu->dec.rt] != 0) {
			cpu->gpr[cpu->dec.rd] = cpu->gpr[cpu->dec.rs];
		}
		break;
	case ((uint32_t) SPECIAL << 26) | MFHI:
		cpu->gpr[cpu->dec.rd] = cpu->hilo.u32[HI];
		break;
	case ((uint32_t) SPECIAL << 26) | MFLO:
		cpu->gpr[cpu->dec.rd] = cpu->hilo.u32[LO];
		break;
	case ((uint32_t) SPECIAL << 26) | MULT:
		cpu->hilo.s64 = (int32_t) cpu->gpr[cpu->dec.rs] *
		    (int32_t) cpu->gpr[cpu->dec.rt];
		break;
	case ((uint32_t) SPECIAL << 26) | DIV:
		cpu->hilo.s32[LO] = ((int32_t) cpu->gpr[cpu->dec.rs] /
		    (int32_t) cpu->gpr[cpu->dec.rt]);
		cpu->hilo.s32[HI] = ((int32_t) cpu->gpr[cpu->dec.rs] %
		    (int32_t) cpu->gpr[cpu->dec.rt]);
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
	case ((uint32_t) SPECIAL << 26) | SLT:
		cpu->gpr[cpu->dec.rd] = ((int32_t) cpu->gpr[cpu->dec.rs]) <
		    ((int32_t) cpu->gpr[cpu->dec.rt]);
		break;
	case ((uint32_t) REGIMM << 26) | (BLTZ << 16):
		if (((int32_t) cpu->gpr[cpu->dec.rs]) < 0) {
			ext = cpu->dec.imm;
			off = ext << 2;
			cpu->dec.npc = cpu->pc + 4 + off;
		} else {
			cpu->dec.isjump = 0;
		}
		break;
	case ((uint32_t) REGIMM << 26) | (BGEZ << 16):
		if (((int32_t) cpu->gpr[cpu->dec.rs]) >= 0) {
			ext = cpu->dec.imm;
			off = ext << 2;
			cpu->dec.npc = cpu->pc + 4 + off;
		} else {
			cpu->dec.isjump = 0;
		}
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
#ifdef VERBOSE
		warnx("%s execute -- cpu[%u] cycle %lu -- going to sleep",
		      __FILE__, cpu->gpr[K0], cpu->perfct.cycle);
#endif
		cpu->running = 0;
		break;
	case ((uint32_t) COP2 << 26) | (MFC2 << 21):
		data = CPU_mfc2(cpu, cpu->dec.rd, cpu->dec.sel);
		if (data < 0) {
			return errno;	/* EINVAL */
		}
		cpu->gpr[cpu->dec.rt] = (uint32_t) data;
		break;
	case ((uint32_t) COP2 << 26) | (MTC2 << 21):
		if ((errnum = CPU_mtc2(cpu, cpu->dec.rd, cpu->dec.sel,
			     cpu->gpr[cpu->dec.rt]))) {
			return errno;	/* EINVAL */
		}
		break;
	case ((uint32_t) COP2 << 26) | (1 << 25): /* coprocessor operation */
		switch (COFUN(cpu->dec.raw)) {
		case MEMSZ:
			cpu->gpr[K1] = mem->size;
			break;
		case CT0:
			cpu->perfct.ct[0].en = !cpu->perfct.ct[0].en;
			break;
		case INPUT:
			CPU_mtc2(cpu, COP2_MSG, COP2_MSG_ST, COP2_MSG_OP_IN);
			++cpu->perfct.nin;
			break;
		case OUTPUT:
			CPU_mtc2(cpu, COP2_MSG, COP2_MSG_ST, COP2_MSG_OP_OUT);
			++cpu->perfct.nout;
			break;
		case ALT:
			CPU_mtc2(cpu, COP2_MSG, COP2_MSG_ST, COP2_MSG_OP_ALT);
			++cpu->perfct.nalt;
			break;
		default:
			return ENOTSUP;
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
		if (addr == IO_ADDR) {
			cpu->gpr[cpu->dec.rt] = (int32_t) getchar();
		} else {
			if (Mem_busacc(cpu->gpr[K0])) {
				cpu->dec.stall = 1;
				++cpu->perfct.lddefer;
#ifdef VERBOSE
				warnx("%s execute -- cpu[%u] cycle %lu -- "
				      "Mem_lb deferred",
				      __FILE__, cpu->gpr[K0],
				      cpu->perfct.cycle);
#endif
				return EBUSY;
			}
			if ((data = Mem_lb(mem, addr)) < 0) {
				return errno;	/* EADDRNOTAVAIL */
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
#ifdef VERBOSE
			warnx("%s execute -- cpu[%u] cycle %lu -- "
			      "Mem_lw deferred",
			      __FILE__, cpu->gpr[K0], cpu->perfct.cycle);
#endif
			return EBUSY;
		}
		if ((data = Mem_lw(mem, addr)) < 0) {
			return errno;	/* EADDRNOTAVAIL, EFAULT */
		}
		cpu->gpr[cpu->dec.rt] = data;
		break;
	case ((uint32_t) SB << 26):
		++cpu->perfct.st;
		if (addr == IO_ADDR) {
			putchar((int) cpu->gpr[cpu->dec.rt]);
		} else if (Mem_busacc(cpu->gpr[K0])) {
			cpu->dec.stall = 1;
			++cpu->perfct.stdefer;
#ifdef VERBOSE
			warnx("%s execute -- cpu[%u] cycle %lu -- "
			      "Mem_sb deferred ",
			      __FILE__, cpu->gpr[K0], cpu->perfct.cycle);
#endif
			return EBUSY;
		} else if ((errnum = Mem_sb(mem, addr,
		           cpu->gpr[cpu->dec.rt]))) {
			return errno;	/* EADDRNOTAVAIL */
		}
		break;
	case ((uint32_t) SW << 26):
		++cpu->perfct.st;
		if (addr == IO_ADDR) {
			printf("%08x\n", cpu->gpr[cpu->dec.rt]);
		} else if (Mem_busacc(cpu->gpr[K0])) {
			cpu->dec.stall = 1;
			++cpu->perfct.stdefer;
#ifdef VERBOSE
			warnx("%s execute -- cpu[%u] cycle %lu -- "
			      "Mem_sw deferred",
			      __FILE__, cpu->gpr[K0], cpu->perfct.cycle);
#endif
			return EBUSY;
		} else if ((errnum = Mem_sw(mem, addr,
		           cpu->gpr[cpu->dec.rt]))) {
			return errno;	/* EADDRNOTAVAIL, EFAULT */
		}
		break;
	case ((uint32_t) LL << 26):
		++cpu->perfct.ll;
		if (Mem_busacc(cpu->gpr[K0])) {
			cpu->dec.stall = 1;
			++cpu->perfct.lldefer;
#ifdef VERBOSE
			warnx("%s execute -- cpu[%u] cycle %lu -- "
			      "Mem_ll deferred",
			      __FILE__, cpu->gpr[K0], cpu->perfct.cycle);
#endif
			return EBUSY;
		}
		if ((data = Mem_ll(mem, cpu->gpr[K0], addr)) < 0) {
			return errno;	/* EOVERFLOW, EADDRNOTAVAIL, EFAULT */
		}
		cpu->gpr[cpu->dec.rt] = data;
		break;
	case ((uint32_t) SC << 26):
		++cpu->perfct.sc;
		if (Mem_busacc(cpu->gpr[K0])) {
			cpu->dec.stall = 1;
			++cpu->perfct.scdefer;
#ifdef VERBOSE
			warnx("%s execute -- cpu[%u] cycle %lu -- "
			      "Mem_sc deferred ",
			      __FILE__, cpu->gpr[K0], cpu->perfct.cycle);
#endif
			return EBUSY;
		}
		errnum = Mem_sc(mem, cpu->gpr[K0], addr,
				cpu->gpr[cpu->dec.rt]);
		switch (errnum) {
		case 0:
			cpu->gpr[cpu->dec.rt] = 1;
			break;
		case EAGAIN:
#ifdef VERBOSE
			warnx("%s execute -- cpu[%u] cycle %lu -- "
			      "Mem_sc rmw fail",
			      __FILE__, cpu->gpr[K0], cpu->perfct.cycle);
#endif
			cpu->dec.rmwfail = 1;
			++cpu->perfct.rmwfail;
			cpu->gpr[cpu->dec.rt] = 0;
			break;
		default:
			return errnum;	/* EOVERFLOW, EAGAIN, EADDRNOTAVAIL,
					 * EFAULT */
		}
		break;
	default:
		return ENOTSUP;
	}

	return 0;
}

/*
 * Datapath_execute: execute a cycle
 *
 * cpu: CPU running at the moment
 * mem: memory that the CPU is using
 *
 * Returns 0 if success, -1 otherwise.
 */
int
Datapath_execute(CPU *cpu, Mem *mem)
{
	int             errnum;
	int             ret;

	size_t          i;

	int64_t         instr;

	errnum = 0;

	if (!cpu->running) {
		return -1;
	}

	cpu->gpr[0] = 0;	/* forces that r0 is zero */

	cpu->dec.stall = 0;
	cpu->dec.rmwfail = 0;

	ret = 0;

	if (CPU_mfc2(cpu, COP2_MSG, COP2_MSG_ST)) {
		++cpu->perfct.commwait;
		goto INC_CYCLE;
	}

	if ((instr = fetch(cpu, mem)) < 0) {
		warnx("Datapath_execute -- cpu[%u] cycle %lu -- fetch failed",
		      cpu->gpr[K0], cpu->perfct.cycle);
		ret = -1;
		goto INC_CYCLE;
	}

#ifdef INSTRDUMP
	write(cpu->debug.fd, &instr, sizeof(uint32_t));
#endif

	cpu->dec.raw = (uint32_t) instr;
	if (decode(&cpu->dec)) {
		warnx("Datapath_execute -- cpu[%u] cycle %lu -- decode failed",
		      cpu->gpr[K0], cpu->perfct.cycle);
		ret = -1;
		goto INC_CYCLE;
	}

	if (((errnum = execute(cpu, mem)) && !cpu->dec.rmwfail &&
	     !cpu->dec.stall)) {
		warnx("Datapath_execute -- cpu[%u] cycle %lu -- "
		      "execution failed: %s",
		      cpu->gpr[K0], cpu->perfct.cycle, strerror(errnum));
		ret = -1;
		goto INC_CYCLE;
	}

	if (cpu->dec.isjump) {
		cpu->pc = cpu->dec.npc;
	} else if (!cpu->dec.stall) {
		cpu->pc += 4;
	}

INC_CYCLE:
	++cpu->perfct.cycle;

	for (i = 0; i < NCOUNTERS; ++i) {
		if (cpu->perfct.ct[i].en) {
			++cpu->perfct.ct[i].ct;
		}
	}

	return ret;
}

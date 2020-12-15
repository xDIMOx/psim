/* Check LICENSE file for copyright and license details. */

/*
 * CPU module
 *
 * Implementation of a MIPS processor.
 *
 * COP2 have one register number (COP2_MSG) that represents the communication
 * link. The link have five registers, selected by the "sel" field:
 *	COP2_CORR is the correspondent processor;
 *	COP2_DATA is the data sent/received;
 *	COP2_STATUS is the status of the link, its format is:
 *		bit 0..1: op (current operation)
 *		bit 2: sent message
 *	COP2_MSG_NMSG is the number of messages received by the processor
 *	COP2_MSG_HOPS is the total number of hops received by the processor
 */

/*
 * Error handling
 */

#define NCOUNTERS 1

#define CPUErrList                               \
X(CPUERR_SUCC, "Success")                        \
X(CPUERR_ALLOC, "Could not allocate CPU")        \
X(CPUERR_DEBUG, "Could not generate debug info") \
X(CPUERR_COP2REG, "Invalid COP2 register")

#define X(a, b) a,
enum CPUErrNo {
	CPUErrList
};
#undef X

int             CPU_errno;

/*
 * Definitions
 */

#define N_GPR 32		/* No. of registers */

#define COP2_NREG 1		/* No. of registers */
#define COP2_NSEL 6		/* select field */

/* TODO: clean way to set these fields */
#define COP2_MSG_ST_OP(x) ((x) & 0x3)	/* operation flag */
#define COP2_MSG_ST_SENT(x) ((x) & 0x4) /* sent flag */

enum CPURegNo {
	ZERO,
	AT,
	V0,
	V1,
	A0,
	A1,
	A2,
	A3,
	T0,
	T1,
	T2,
	T3,
	T4,
	T5,
	T6,
	T7,
	S0,
	S1,
	S2,
	S3,
	S4,
	S5,
	S6,
	S7,
	T8,
	T9,
	K0,
	K1,
	GP,
	SP,
	FP,
	RA
};

enum hilo {
	LO,
	HI,
};

enum cop2_reg {
	COP2_MSG,
};

enum cop2_link_sel {
	COP2_MSG_CORR,
	COP2_MSG_DATA,
	COP2_MSG_ST,
	COP2_MSG_NMSG,
	COP2_MSG_HOPS,
};

enum cop2_link_op {
	COP2_MSG_OP_NONE,
	COP2_MSG_OP_IN,
	COP2_MSG_OP_OUT,
};

typedef struct {
	uint32_t        raw;
	uint32_t        sign;
	int16_t         imm;
	uint8_t         ismem;
	uint8_t         isjump;
	uint8_t         sa;
	uint8_t         rd;
	uint8_t         rs, rt;
	uint32_t        idx;
	uint32_t        npc;
	uint32_t        stall;
	uint32_t        sel;
} Decoder;

typedef struct {
	size_t          pc;		/* Program counter */
	uint32_t        gpr[N_GPR];	/* General purpose registers */
	union {
		int64_t         s64;
		uint64_t        u64;
		int32_t         s32[2];
		uint32_t        u32[2];
	}               hilo;	/* HI/LO */
	Decoder         dec;	/* instruction decoder */
	uint32_t        cop2[COP2_NREG][COP2_NSEL];
	struct {
		size_t          cycle;		/* cycles executed */
		size_t          ld;		/* no. of loads */
		size_t          lddefer;	/* no. of deferred loads */
		size_t          st;		/* no. of stores */
		size_t          stdefer;	/* no. of deferred stores */
		size_t          ll;		/* no. of load linked
						 * instructions */
		size_t          lldefer;	/* no. of deferred ll's */
		size_t          sc;		/* no. of store-conditional
						 * instructions */
		size_t          scdefer;	/* no. of deferred sc's */
		size_t          rmwfail;	/* no. of RMW failures */
		size_t          nin;		/* no. of inputs */
		size_t          nout;		/* no. of outputs */
		size_t          commwait;	/* cycles waiting to
						 * communication to finish */
		struct {
			size_t          en;	/* enable */
			size_t          ct;	/* cycles counted */
		}               ct[NCOUNTERS];
	}               perfct;	/* performance counters */
#ifndef NDEBUG
	struct {
		int             fd;
		char            fname[20];
	}               debug;
#endif
} CPU;

CPU            *CPU_create(uint32_t);
void            CPU_destroy(CPU *);

void            CPU_setpc(CPU *, uint32_t);

int64_t         CPU_mfc2(CPU *, uint32_t, uint32_t);
int             CPU_mtc2(CPU *, uint32_t, uint32_t, uint32_t);

const char     *CPU_strerror(int);

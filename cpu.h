/* Check LICENSE file for copyright and license details. */

/*
 * CPU module
 *
 * Implementation a MIPS processor
 */

/*
 * Error handling
 */

#define CPUErrList                        \
X(CPUERR_SUCC, "Success")                 \
X(CPUERR_ALLOC, "Could not allocate CPU") \
X(CPUERR_DEBUG, "Could not generate debug info")

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
} Decoder;

typedef struct {
	uint32_t        pc;	/* Program counter */
	uint32_t        gpr[N_GPR];	/* General purpose registers */
	Decoder         dec;	/* instruction decoder */

	size_t          cycle;
	size_t          ld;
	size_t          st;
	size_t          memfail;
	size_t          ll;
	size_t          sc;
	size_t          rmwfail;
#ifndef NDEBUG
	struct {
		int             fd;
		char            fname[20];
	}               debug;
#endif
} CPU;

CPU            *CPU_create(uint32_t id);
void            CPU_destroy(CPU *cpu);

const char     *CPU_strerror(int errno);

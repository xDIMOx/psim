/* Check LICENSE file for copyright and license details. */

/*
 * CPU module
 *
 * Implementation a MIPS processor
 */

/*
 * Error handling
 */

#define NLOCKPERF 6
#define NCOUNTERS 1

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

enum hilo {
	LO,
	HI,
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
	union {
		int64_t s64;
		int32_t s32[2];
		uint64_t u64;
		uint32_t u32[2];

	} hilo; /* HI/LO */
	Decoder         dec;	/* instruction decoder */

	struct {
		size_t          cycle;	/* cycles executed */
		size_t          ld;	/* no. of loads */
		size_t          lddefer;	/* no. of deferred loads */
		size_t          st;	/* no. of stores */
		size_t          stdefer;	/* no. of deferred stores */
		size_t          ll;	/* no. of load linked instructions */
		size_t          lldefer;	/* no. of deferred ll's */
		size_t          sc;	/* no. of store-conditional
					 * instructions */
		size_t          scdefer;	/* no. of deferred sc's */
		size_t          rmwfail;	/* no. of RMW failures */
		struct {
			size_t          en;	/* enable */
			size_t          ct;	/* cycles counted */
		}               ct[NCOUNTERS];
		struct {
			size_t          en;	/* enabled */
			size_t          cycle;	/* cycles counted */
			size_t          acc;	/* no. of accesses */
		}               lockperf[NLOCKPERF];	/* locks performance
							 * counters */
	}               perfct;	/* performance counters */
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

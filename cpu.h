/* Check LICENSE file for copyright and license details. */

/*
 * CPU module
 *
 * Implementation a MIPS processor
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
	uint32_t        pc;	/* Program counter */
	uint32_t        gpr[N_GPR];	/* General purpose registers */
} CPU;

CPU            *CPU_create(void);
void            CPU_destroy(CPU *cpu);

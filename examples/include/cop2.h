/* Check LICENSE file for copyright and license details. */

enum Cop2Op {			/* Coprocessor 2 operation */
	MEMSZ,
	CT0,
	INPUT,
	OUTPUT,
	ALT,
};

/* Coprocessor 2 registers */
#define COMM $0

enum Cop2$0Sel {		/* Coprocessor 2 $0 registers (select) */
	CORR,
	DATA,
	ST,
	HOPS,
	NMSG,
	NCL,
};

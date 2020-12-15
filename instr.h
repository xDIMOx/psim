/* Check LICENSE file for copyright and license details. */

/*
 * CPU instruction codes and helper macros
 */

#define OPC(i) (((i) & 0xFC000000))

#define FUNC(i) ((i) & 0x3F)
#define TF(i) ((i) & 0x100)
#define SHROT(i) ((i) & 0x200000)
#define SHROTV(i) ((i) & 0x40)

#define SA(i) (((i) & 0x7C0))

#define IMM(i) ((int16_t) ((i) & 0xFFFF))

#define INSTR_IDX(i) ((i) & 0x3FFFFFF)

#define RD(i) (((i) & 0xF800))
#define RT(i) (((i) & 0x1F0000))
#define RS(i) (((i) & 0x3E00000))

#define CO(i) ((i) & 0x02000000)
#define COFUN(i) ((i) & 0x01FFFFFF)
#define SEL(i) FUNC(i)

#define LSB(i) (((i) >> 6) & 0x3F)
#define MSBD(i) (((i) >> 11) & 0x3F)

enum opcodes {
	SPECIAL,
	REGIMM,
	J,
	JAL,
	BEQ,
	BNE,
	BLEZ,
	BGTZ,
	ADDI,
	ADDIU,
	SLTI,
	SLTIU,
	ANDI,
	ORI,
	XORI,
	LUI,
	COP0,
	COP1,
	COP2,
	COP1X,
	BEQL,
	BNEL,
	BLEZL,
	BGTZL,
	OPC_IGN0,
	OPC_IGN1,
	OPC_IGN2,
	OPC_IGN3,
	SPECIAL2,
	JALX,
	OPC_IGN4,
	SPECIAL3,
	LB,
	LH,
	LWL,
	LW,
	LBU,
	LHU,
	LWR,
	OPC_IGN5,
	SB,
	SH,
	SWL,
	SW,
	OPC_IGN6,
	OPC_IGN7,
	SWR,
	CACHE,
	LL,
	LWC1,
	LWC2,
	PREF,
	OPC_IGN8,
	LDC1,
	LDC2,
	OPC_IGN9,
	SC,
	SWC1,
	SWC2,
	OPC_IGN10,
	OPC_IGN11,
	SDC1,
	SDC2,
	OPC_IGN12,
};

enum special_func {
	SLL,
	MOVCI,
	SRL_FIELD,
	SRA,
	SLLV,
	FUNC_IGN0,
	SRLV_FIELD,
	SRAV,
	JR,
	JALR,
	MOVZ,
	MOVN,
	SYSCALL,
	BREAK,
	FUNC_IGN1,
	SYNC,
	MFHI,
	MTHI,
	MFLO,
	MTLO,
	FUNC_IGN2,
	FUNC_IGN3,
	FUNC_IGN4,
	FUNC_IGN5,
	MULT,
	MULTU,
	DIV,
	DIVU,
	FUNC_IGN6,
	FUNC_IGN7,
	FUNC_IGN8,
	FUNC_IGN9,
	ADD,
	ADDU,
	SUB,
	SUBU,
	AND,
	OR,
	XOR,
	NOR,
	FUNC_IGN10,
	FUNC_IGN11,
	SLT,
	SLTU,
	FUNC_IGN12,
	FUNC_IGN13,
	FUNC_IGN14,
	FUNC_IGN15,
	TGE,
	TGEU,
	TLT,
	TLTU,
	TEQ,
	FUNC_IGN16,
	TNE,
	FUNC_IGN17,
	FUNC_IGN18,
	FUNC_IGN19,
	FUNC_IGN20,
	FUNC_IGN21,
	FUNC_IGN22,
	FUNC_IGN23,
	FUNC_IGN24,
	FUNC_IGN25,
};

enum regimm_rt {
	BLTZ,
	BGEZ,
	BLTZL,
	BGEZL,
	RT_IGN0,
	RT_IGN1,
	RT_IGN2,
	RT_IGN3,
	TGEI,
	TGEIU,
	TLTI,
	TLTIU,
	TEQI,
	RT_IGN4,
	TNEI,
	RT_IGN5,
	BLTZAL,
	BGEZAL,
	BLTZALL,
	BGEZALL,
	RT_IGN6,
	RT_IGN7,
	RT_IGN8,
	RT_IGN9,
	RT_IGN10,
	RT_IGN11,
	RT_IGN12,
	RT_IGN13,
	RT_IGN14,
	RT_IGN15,
	RT_IGN16,
	SYNCI,
};

enum special2_func {
	MADD,
	MADDU,
	MUL,
	SPECIAL2_IGN0,
	MSUB,
	MSUBU,
	SPECIAL2_IGN1,
	SPECIAL2_IGN2,
	SPECIAL2_IGN3,
	SPECIAL2_IGN4,
	SPECIAL2_IGN5,
	SPECIAL2_IGN6,
	SPECIAL2_IGN7,
	SPECIAL2_IGN8,
	SPECIAL2_IGN9,
	SPECIAL2_IGN10,
	SPECIAL2_IGN11,
	SPECIAL2_IGN12,
	SPECIAL2_IGN13,
	SPECIAL2_IGN14,
	SPECIAL2_IGN15,
	SPECIAL2_IGN16,
	SPECIAL2_IGN17,
	SPECIAL2_IGN18,
	SPECIAL2_IGN19,
	SPECIAL2_IGN20,
	SPECIAL2_IGN21,
	SPECIAL2_IGN22,
	SPECIAL2_IGN23,
	SPECIAL2_IGN24,
	SPECIAL2_IGN25,
	SPECIAL2_IGN26,
	CLZ,
	CLO,
	SPECIAL2_IGN27,
	SPECIAL2_IGN28,
	SPECIAL2_IGN29,
	SPECIAL2_IGN30,
	SPECIAL2_IGN31,
	SPECIAL2_IGN32,
	SPECIAL2_IGN33,
	SPECIAL2_IGN34,
	SPECIAL2_IGN35,
	SPECIAL2_IGN36,
	SPECIAL2_IGN37,
	SPECIAL2_IGN38,
	SPECIAL2_IGN39,
	SPECIAL2_IGN40,
	SPECIAL2_IGN41,
	SPECIAL2_IGN42,
	SPECIAL2_IGN43,
	SPECIAL2_IGN44,
	SPECIAL2_IGN45,
	SPECIAL2_IGN46,
	SPECIAL2_IGN47,
	SPECIAL2_IGN48,
	SPECIAL2_IGN49,
	SPECIAL2_IGN50,
	SPECIAL2_IGN51,
	SPECIAL2_IGN52,
	SPECIAL2_IGN53,
	SPECIAL2_IGN54,
	SPECIAL2_IGN55,
	SDBBP,
};

enum special3_func {
	EXT,
	SPECIAL3_IGN0,
	SPECIAL3_IGN1,
	SPECIAL3_IGN2,
	INS,
	SPECIAL3_IGN4,
	SPECIAL3_IGN5,
	SPECIAL3_IGN6,
	SPECIAL3_IGN7,
	SPECIAL3_IGN8,
	SPECIAL3_IGN9,
	SPECIAL3_IGN10,
	SPECIAL3_IGN11,
	SPECIAL3_IGN12,
	SPECIAL3_IGN13,
	SPECIAL3_IGN14,
	SPECIAL3_IGN15,
	SPECIAL3_IGN16,
	SPECIAL3_IGN17,
	SPECIAL3_IGN18,
	SPECIAL3_IGN19,
	SPECIAL3_IGN20,
	SPECIAL3_IGN21,
	SPECIAL3_IGN22,
	SPECIAL3_IGN23,
	SPECIAL3_IGN24,
	SPECIAL3_IGN25,
	SPECIAL3_IGN26,
	SPECIAL3_IGN27,
	SPECIAL3_IGN28,
	SPECIAL3_IGN29,
	SPECIAL3_IGN30,
	BSHFL,
	SPECIAL3_IGN31,
	SPECIAL3_IGN32,
	SPECIAL3_IGN33,
	SPECIAL3_IGN34,
	SPECIAL3_IGN35,
	SPECIAL3_IGN36,
	SPECIAL3_IGN37,
	SPECIAL3_IGN38,
	SPECIAL3_IGN39,
	SPECIAL3_IGN40,
	SPECIAL3_IGN41,
	SPECIAL3_IGN42,
	SPECIAL3_IGN43,
	SPECIAL3_IGN44,
	SPECIAL3_IGN46,
	SPECIAL3_IGN47,
	SPECIAL3_IGN48,
	SPECIAL3_IGN49,
	SPECIAL3_IGN50,
	SPECIAL3_IGN51,
	SPECIAL3_IGN52,
	SPECIAL3_IGN53,
	SPECIAL3_IGN54,
	SPECIAL3_IGN55,
	SPECIAL3_IGN56,
	SPECIAL3_IGN57,
	RDHWR,
	SPECIAL3_IGN58,
	SPECIAL3_IGN59,
	SPECIAL3_IGN60,
	SPECIAL3_IGN61,
};

enum movci {
	MOVF,
	MOVT,
};

enum srl {
	SRL,
	ROTR,
};

enum srlv {
	SRLV,
	ROTRV,
};

enum bshfl {
	BSHFL_IGN0,
	BSHFL_IGN1,
	WSBH,
	BSHFL_IGN2,
	BSHFL_IGN3,
	BSHFL_IGN4,
	BSHFL_IGN5,
	BSHFL_IGN6,
	BSHFL_IGN7,
	BSHFL_IGN8,
	BSHFL_IGN9,
	BSHFL_IGN10,
	BSHFL_IGN11,
	BSHFL_IGN12,
	BSHFL_IGN13,
	BSHFL_IGN14,
	SEB,
	BSHFL_IGN15,
	BSHFL_IGN16,
	BSHFL_IGN17,
	BSHFL_IGN18,
	BSHFL_IGN19,
	BSHFL_IGN20,
	BSHFL_IGN21,
	SEH,
	BSHFL_IGN22,
	BSHFL_IGN23,
	BSHFL_IGN24,
	BSHFL_IGN25,
	BSHFL_IGN26,
	BSHFL_IGN27,
	BSHFL_IGN28,
};

enum cop0_rs {
	MFC0,
	COP0RS_IGN0,
	COP0RS_IGN1,
	COP0RS_IGN2,
	MTC0,
	COP0RS_IGN3,
	COP0RS_IGN4,
	COP0RS_IGN5,
	COP0RS_IGN6,
	COP0RS_IGN7,
	RDPGPR,
	MFMC0,
	COP0RS_IGN8,
	COP0RS_IGN9,
	WRPGPR,
	COP0RS_IGN10,		/* For values after this, it encodes the C0
				 * value */
};

enum cop0_co {
	C0_IGN0,
	TLBR,
	TLBWI,
	C0_IGN1,
	C0_IGN2,
	C0_IGN3,
	TLBWR,
	C0_IGN4,
	TLBP,
	C0_IGN5,
	C0_IGN6,
	C0_IGN7,
	C0_IGN8,
	C0_IGN9,
	C0_IGN10,
	C0_IGN11,
	C0_IGN12,
	C0_IGN13,
	C0_IGN14,
	C0_IGN15,
	C0_IGN16,
	C0_IGN17,
	C0_IGN18,
	C0_IGN19,
	ERET,
	C0_IGN20,
	C0_IGN21,
	C0_IGN22,
	C0_IGN23,
	C0_IGN24,
	C0_IGN25,
	DERET,
	WAIT,
	C0_IGN26,
	C0_IGN27,
	C0_IGN28,
	C0_IGN29,
	C0_IGN30,
	C0_IGN31,
	C0_IGN32,
	C0_IGN34,
	C0_IGN35,
	C0_IGN36,
	C0_IGN37,
	C0_IGN38,
	C0_IGN39,
	C0_IGN40,
	C0_IGN41,
	C0_IGN42,
	C0_IGN43,
	C0_IGN44,
	C0_IGN45,
	C0_IGN46,
	C0_IGN47,
	C0_IGN48,
	C0_IGN49,
	C0_IGN50,
	C0_IGN51,
	C0_IGN52,
	C0_IGN53,
	C0_IGN54,
	C0_IGN55,
	C0_IGN56,
	C0_IGN57,
};

enum cop2_rs {
	MFC2,
	COP2RS_IGN0,
	CFC2,
	MFHC2,
	MTC2,
	COP2RS_IGN1,
	CTC2,
	MTHC2,
	BC2,
	COP2RS_IGN2,
	COP2RS_IGN3,
	COP2RS_IGN4,
	COP2RS_IGN5,
	COP2RS_IGN6,
	COP2RS_IGN7,
	COP2RS_IGN8,
};

enum cofun2 {
	MEMSZ,			/* memory size */
	CT0,			/* counter */
	INPUT,			/* input */
	OUTPUT,			/* output */
};

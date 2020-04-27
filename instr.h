/* Check LICENSE file for copyright and license details. */

/*
 * CPU instruction codes and helper macros
 */

#define OPC(i) (((i) & 0xFC000000))

#define FUNC(i) ((i) & 0x3F)
#define TF(i) ((i) & 0x100)
#define SHROT(i) ((i) & 0x200000)
#define SHROTV(i) ((i) & 0x40)

#define IMM(i) ((int16_t) ((i) & 0xFFFF))

#define RD(i) (((i) & 0xF800))
#define RT(i) (((i) & 0x1F0000))
#define RS(i) (((i) & 0x3E00000))

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

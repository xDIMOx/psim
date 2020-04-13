/* Check LICENSE file for copyright and license details. */

/*
 * Processor's datapath
 */

/* Instruction decoder */
typedef struct {
	uint32_t        raw;
	uint32_t        sign;	/* instruction signature */
	const char     *name;
	int32_t         isjump;	/* instruction is a jump */
	uint32_t        pc;
	uint32_t        npc;
	uint8_t         rd;
	uint8_t         rs, rt;
	uint8_t         sa;
	int16_t         imm;
	int16_t         ismem;	/* the intruction uses the memory */
	uint32_t        idx;
	uint32_t        region;
	uint32_t        addr;	/* address to be acessed */
	uint32_t        sel;	/* coprocessor select */
	uint32_t        cofun;	/* coprocessor operation */
} Decoder;

int             Datapath_execute(CPU *cpu, Mem *mem);

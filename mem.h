/* Check LICENSE file for copyright and license details. */

/*
 * Memory module
 */

/*
 * Error handling
 */

#define MemErrList                                    \
X(MEMERR_SUCC, "Success")                             \
X(MEMERR_ALLOC, "Could not allocate memory")          \
X(MEMERR_BND, "Address is out of bounds")             \
X(MEMERR_ALIGN, "Misaligned address")                 \
X(MEMERR_SHR, "Processor can not used shared memory") \
X(MEMERR_SC, "SC failed")

#define X(a, b) a,
enum MemErrNo {
	MemErrList
};
#undef X

int             Mem_errno;

/*
 * Definitions
 */

typedef struct {
	size_t          size;
	union {
		uint8_t        *b;	/* Bytes */
		uint32_t       *w;	/* Words */
	}               data;
} Mem;

Mem            *Mem_create(size_t size, size_t nshr);
void            Mem_destroy(Mem *mem);

int             Mem_busacc(uint32_t prid);
void            Mem_busclr(void);
size_t          Mem_busutil(void);

int             Mem_progld(Mem *mem, unsigned char *elf);

int64_t         Mem_lw(Mem *mem, size_t addr);
int             Mem_sw(Mem *mem, size_t addr, uint32_t data);

int64_t         Mem_ll(Mem *mem, uint32_t prid, size_t addr);
int             Mem_sc(Mem *mem, uint32_t prid, size_t addr, uint32_t data);

int64_t         Mem_lb(Mem *mem, size_t addr);
int             Mem_sb(Mem *mem, size_t addr, uint8_t data);

const char     *Mem_strerror(int code);

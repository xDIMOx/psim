/* Check LICENSE file for copyright and license details. */

/*
 * Memory module
 */

/*
 * Error handling
 */

#define MemErrList                           \
X(MEMERR_SUCC, "Success")                    \
X(MEMERR_ALLOC, "Could not allocate memory") \
X(MEMERR_BND, "Address is out of bounds")    \
X(MEMERR_ALIGN, "Misaligned address")

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

Mem            *Mem_create(size_t size);
void            Mem_destroy(Mem *mem);

int             Mem_progld(Mem *mem, unsigned char *elf);

int64_t         Mem_lw(Mem *mem, size_t addr);
int             Mem_sw(Mem *mem, size_t addr, uint32_t data);

const char     *Mem_strerror(int errno);

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

Mem            *Mem_create(size_t, size_t);
void            Mem_destroy(Mem *);

int             Mem_busacc(uint32_t);
void            Mem_busclr(void);
size_t          Mem_busutil(void);

int             Mem_progld(Mem *, unsigned char *);

int64_t         Mem_lw(Mem *, size_t);
int             Mem_sw(Mem *, size_t, uint32_t);

int64_t         Mem_ll(Mem *, uint32_t prid, size_t);
int             Mem_sc(Mem *, uint32_t prid, size_t, uint32_t);

int64_t         Mem_lb(Mem *, size_t);
int             Mem_sb(Mem *, size_t, uint8_t);

uint32_t       *Mem_getptr(Mem *, uint32_t);

const char     *Mem_strerror(int);

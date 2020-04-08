/* Check LICENSE file for copyright and license details. */

/*
 * Memory module
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

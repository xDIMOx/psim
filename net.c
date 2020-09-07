/* Check LICENSE file for copyright and license details. */

/*
 * Network node
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "mem.h"

/* Implements */
#include "net.h"

#define X(a, b) b,
static const char *Net_errlist[] = {
	NetErrList
};
#undef X

Net            *Net_create(size_t x, size_t y, size_t memsz);
void            Net_destroy(Net *net);

const char     *Net_strerror(int code);

/*
 * Net_create: create cpu network
 *
 * x: number of processors on the x axis
 * y: number of processors on the y axis
 * memsz: memory size
 *
 * Returns network if success, NULL otherwise
 */
Net            *
Net_create(size_t x, size_t y, size_t memsz)
{
	size_t          i;

	Net            *net;

	Net_errno = NETERR_SUCC;

	if (!(net = malloc(sizeof(Net)))) {
		Net_errno = NETERR_ALLOC;
		return NULL;
	}
	net->cycle = 0;

	net->size = x * y;
	if (!(net->nd = malloc(sizeof(struct node) * net->size))) {
		Net_errno = NETERR_ALLOC;
		return NULL;
	}
	for (i = 0; i < net->size; ++i) {
		if (!(net->nd[i].cpu = CPU_create(i))) {
			Net_errno = NETERR_CPU;
			return NULL;
		}
		if (!(net->nd[i].mem = Mem_create(memsz, 1))) {
			Net_errno = NETERR_MEM;
			return NULL;
		}
	}

	return net;
}

/*
 * Net_destroy: deallocate network object
 */
void
Net_destroy(Net *net)
{
	size_t          i;

	for (i = 0; i < net->size; ++i) {
		CPU_destroy(net->nd[i].cpu);
		Mem_destroy(net->nd[i].mem);
	}
}

/*
 * Net_strerror: map error number to error message string
 *
 * code: error number
 *
 * Returns error message string
 */
inline const char *
Net_strerror(int code)
{
	return Net_errlist[code];
}

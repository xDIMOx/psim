/* Check LICENSE file for copyright and license details. */

/*
 * Dining philosophers
 */

#include "common.h"
#include "semaphore.h"

#ifndef PHILOS
#define PHILOS 5
#elif PHILOS <= 0
#error "PHILOS has to be > 0"
#endif

#ifndef IDEAS
#define IDEAS 1
#endif

#ifndef THINK
#define THINK 1
#endif

#ifndef EAT
#define EAT 1
#endif

static int      footman = PHILOS - 1;
static int      fork[PHILOS];
static int      lock = 1;

int
main(void)
{
	int             flag;

	unsigned int    id;
	unsigned int    fid;
	unsigned int    ideas;

	id = processor_id();
	fork[id] = 1;

	/* garantee that randu starts with odd value */
	if (id & 1) {
		randstate = id * 3;
	} else {
		randstate = id + 1;
	}

	fid = rem(id + 1, PHILOS);
	for (ideas = IDEAS; ideas > 0; --ideas) {
		busywait(THINK);
		Sem_P(&footman);
		Sem_P(&fork[id]);
		Sem_P(&fork[fid]);
		busywait(EAT);
		Sem_V(&fork[fid]);
		Sem_V(&fork[id]);
		Sem_V(&footman);
	}

out:
	return 0;
}

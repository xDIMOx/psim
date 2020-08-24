/* Check LICENSE file for copyright and license details. */

/*
 * Dining philosophers
 */

#include "common.h"
#include "spinlock.h"

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
static int      philos = PHILOS;
static int      lock = 1;

int
main(void)
{
	int             flag;

	unsigned int    id;
	unsigned int    fid;
	unsigned int    ideas;
	unsigned int    t;

	id = thread_id();
	fork[id] = 1;

	ideas = IDEAS;
	fid = rem(id + 1, PHILOS);
	for (;;) {
		busywait(THINK);
		--ideas;
		Spin_lock(&footman);
		Spin_lock(&fork[id]);
		Spin_lock(&fork[fid]);
		busywait(EAT);
		Spin_unlock(&fork[fid]);
		Spin_unlock(&fork[id]);
		Spin_unlock(&footman);
		if (!ideas) {
			Spin_lock(&lock);
			--philos;
			Spin_unlock(&lock);
			goto out;
		}
	}

out:
	flag = (id == 0) ? 1 : 0;
	while (flag) {
		Spin_lock(&lock);
		if (!philos)
			flag = 0;
		Spin_unlock(&lock);
	}

	return 0;
}

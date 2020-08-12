/* Check LICENSE file for copyright and license details. */

/*
 * Dining philosophers
 */

#include "common.h"
#include "spinlock.h"

#ifndef IDEAS
#define IDEAS 1
#elif IDEAS <= 0
#error "IDEAS has to be > 0"
#endif

static int      footman = 4;
static int      fork[5] = {1, 1, 1, 1, 1};
static int      philos = 5;
static int      lock = 1;

int
main(void)
{
	int             flag;

	unsigned int    id;
	unsigned int    ideas;

	volatile long   eat;
	volatile long   think;

	id = thread_id();

	ideas = IDEAS;
	for (;;) {
		for (think = 32 + (randu() & 15); think > 0; --think);
		--ideas;
		Spin_lock(&footman);
		Spin_lock(&fork[id]);
		Spin_lock(&fork[(id < 4) ? id + 1 : 0]);
		for (eat = 32 + (randu() & 15); eat > 0; --eat);
		Spin_unlock(&fork[(id < 4) ? id + 1 : 0]);
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

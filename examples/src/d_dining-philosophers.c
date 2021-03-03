/* Check LICENSE file for copyright and license details. */

/*
 * Dining philosophers
 */

#include "comm.h"
#include "common.h"

#define PHILOS 5

#ifndef IDEAS
#define IDEAS 1
#endif

#ifndef THINK
#define THINK 1
#endif

#ifndef EAT
#define EAT 1
#endif

#define FOOTMAN 2

enum Actions {
	SITDOWN,
	GETUP,
	PICKUP,
	PUTDOWN,
	DONE,
};

enum Philos {
	PHIL0 = 0,
	PHIL1 = 1,
	PHIL2 = 3,
	PHIL3 = 4,
	PHIL4 = 5,
};

enum Forks {
	FORK0 = 6,
	FORK1 = 7,
	FORK2 = 9,
	FORK3 = 10,
	FORK4 = 11,
};

static enum Philos philo[PHILOS] = {PHIL0, PHIL1, PHIL2, PHIL3, PHIL4};

static enum Forks fork[PHILOS] = {FORK0, FORK1, FORK2, FORK3, FORK4};

static struct {
	int             seated;
	int             arr[PHILOS];
}               table;

extern int      randuseed;

void
sitdown(int new)
{
	table.arr[table.seated++] = new;
}

void
getup(int exiting)
{
	int             i;

	for (i = 0; i < table.seated; ++i) {
		if (table.arr[i] == exiting) {
			table.arr[i] = table.arr[table.seated - 1];
			--table.seated;
			return;
		}
	}
}

void
footman(void)
{
	int             np;
	int             ret;
	int             action;

	np = PHILOS;

	table.seated = 0;

	while (np > 0) {
		if (table.seated == (PHILOS - 1)) {
			ret = C2_alt((int *) table.arr, table.seated, &action);
			if (action == GETUP) {
				getup(ret);
			}
		} else {
			ret = C2_alt((int *) philo, PHILOS, &action);
			switch (action) {
			case SITDOWN:
				sitdown(ret);
				break;
			case GETUP:
				getup(ret);
				break;
			case DONE:
				--np;
				break;
			}
		}
	}
}

void
cutlery(int i)
{
	int             np;
	int             aux;
	int             ret;
	int             action;

	int             allowed[2];

	allowed[0] = philo[i];
	if ((aux = i - 1) < 0) {
		allowed[1] = philo[PHILOS - 1];
	} else {
		allowed[1] = philo[aux];
	}

	np = 2;
	while (np > 0) {
		ret = C2_alt((int *) allowed, 2, &action);
		printhex(ret);
		switch (action) {
		case PICKUP:
			while (C2_input(ret) != PUTDOWN);
			break;
		case DONE:
			--np;
			break;
		}
	}
}

void
philosopher(int i)
{
	int             ideas;
	int             ownfork, nbrfork;

	ownfork = fork[i];
	nbrfork = fork[rem(i + 1, PHILOS)];

	for (ideas = IDEAS; ideas > 0; --ideas) {
		busywait(THINK);
		C2_output(FOOTMAN, SITDOWN);
		C2_output(ownfork, PICKUP);
		C2_output(nbrfork, PICKUP);
		busywait(EAT);
		C2_output(nbrfork, PUTDOWN);
		C2_output(ownfork, PUTDOWN);
		C2_output(FOOTMAN, GETUP);
	}

	C2_output(FOOTMAN, DONE);
	C2_output(ownfork, DONE);
	C2_output(nbrfork, DONE);
}

int
main(void)
{
	int             i;
	int             id;

	id = thread_id();

	if (id & 1) {
		randuseed = id * 3;
	} else {
		randuseed = (id + 1) * 3;
	}

	switch (id) {
	case PHIL0:
	case PHIL1:
	case PHIL2:
	case PHIL3:
	case PHIL4:
		for (i = 0; i < PHILOS; ++i) {
			if (philo[i] == id) {
				philosopher(i);
			}
		}
		break;
	case FOOTMAN:
		footman();
		break;
	case FORK0:
	case FORK1:
	case FORK2:
	case FORK3:
	case FORK4:
		for (i = 0; i < PHILOS; ++i) {
			if (fork[i] == id) {
				cutlery(i);
			}
		}
		break;
	}
}

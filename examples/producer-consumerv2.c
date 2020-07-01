/* Check LICENSE file for copyright and license details. */

/*
 * Producer-consumer example
 */

#include "common.h"
#include "spinlock.h"

#ifndef MAXELEM
#error "You need to define MAXELEM"
#elif MAXELEM & 2 != 0
#error "MAXELEM has to be a power of 2"	/* to simplify the modulo operation */
#endif

#ifndef MAXVAL
#error "You need to define MAXVAL"
#endif

#ifndef NCONSUMERS
#error "You need to define NCONSUMERS"
#endif

enum flag {
	SUCCESS,
	EMPTY,
	FULL,
	END,
};

static int      ct = 0;
static int      hd;		/* queue's head */
static int      tl;		/* queue's tail */
static int      buf[MAXELEM];
static int      nconsumers = NCONSUMERS;
static int      nempty;
static int      producing = 1;
static int      lock = 1;
static int      full = 0;
static int      empty = 0;

void
enqueue(int item)
{
	buf[tl] = item;
	tl = (tl + 1) & (MAXELEM - 1);
	++ct;
}

int
dequeue(void)
{
	int             item;

	item = buf[hd];
	hd = (hd + 1) & (MAXELEM - 1);
	--ct;

	return item;
}

void
producer(void)
{
	int             val;
	int             i;
	int             flag;
	int             ct;

	switch(thread_id()) {
	case 0:
		val = 0;
		i = 2;
		break;
	case 3:
		val = 1;
		i = 1;
		break;
	}

	while (val < MAXVAL) {
		for (ct = val; ct > 0; --ct);	/* producing */
		Spin_lock(&lock);
		if (ct < MAXELEM) {
			enqueue(val);
			if (nempty > 0) {
				flag = EMPTY;
				--nempty;
			} else
				flag = SUCCESS;
		} else
			flag = FULL;
		Spin_unlock(&lock);
		switch (flag) {
		case SUCCESS:
			break;
		case EMPTY:
			Spin_unlock(&empty);
			break;
		case FULL:
			Spin_lock(&full);
			Spin_lock(&lock);
			enqueue(val);
			Spin_unlock(&lock);
			break;
		}
		val += i;
	}

	Spin_lock(&lock);
	producing = 0;
	Spin_unlock(&lock);

	while (flag != END) {
		Spin_lock(&lock);
		if (!nconsumers)
			flag = END;
		else if (nempty > 0) {
			--nempty;
			Spin_unlock(&empty);
		}
		Spin_unlock(&lock);
	}
}

void
consumer(void)
{
	int             item;
	int             flag;

	flag = SUCCESS;
	while (flag != END) {
		Spin_lock(&lock);
		if (ct > 0) {
			if (ct == MAXELEM)
				flag = FULL;
			else
				flag = SUCCESS;
			item = dequeue();
		} else if (!producing) {
			--nconsumers;
			item = -1;
			flag = END;
		} else {
			++nempty;
			flag = EMPTY;
		}
		Spin_unlock(&lock);
		switch (flag) {
		case SUCCESS:
			break;
		case EMPTY:
			Spin_lock(&empty);
			break;
		case FULL:
			Spin_unlock(&full);
			break;
		}
		while (item-- > 0);	/* consume the data */
	}
}

int
main(void)
{
	unsigned int    id;

	id = thread_id();

	if (id == 0 || id == 3)
		producer();
	else
		consumer();

	return 0;
}

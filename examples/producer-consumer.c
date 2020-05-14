/* Check LICENSE file for copyright and license details. */

/*
 * Producer-consumer example
 */

#include "common.h"
#include "spinlock.h"

#define MAXELEM 1		/* has to be a power of 2, to simplify the
				 * modulo operation */
#define MAXVAL MAXELEM		/* maximum value to be produced */

#define NCONSUMERS 1

enum flag {
	SUCCESS,
	EMPTY,
	FULL,
	END,
};

static int      ct;
static int      hd;		/* queue's head */
static int      tl;		/* queue's tail */
static int      buf[MAXELEM];
static int      nconsumers = NCONSUMERS;
static int      producing;
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
	int             i;
	int             flag;

	Spin_lock(&lock);
	ct = 0;
	producing = 1;
	Spin_unlock(&lock);

	for (i = 0; i < MAXVAL; ++i) {
		Spin_lock(&lock);
		if (ct < MAXELEM) {
			if (ct == 0)
				flag = EMPTY;
			else
				flag = SUCCESS;
			enqueue(i);
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
			enqueue(i);
			Spin_unlock(&lock);
			break;
		}
	}

	while (flag != END) {
		Spin_lock(&lock);
		if (!nconsumers)
			flag = END;
		Spin_unlock(&lock);
	}
}

void
consumer(void)
{
	int             item;
	int             flag;

	Spin_lock(&empty);

	while (flag != END) {
		Spin_lock(&lock);
		if (ct > 0) {
			if (ct == MAXELEM)
				flag = FULL;
			else
				flag = SUCCESS;
			item = dequeue();
		} else if (!producing) {
			item = -1;
			flag = END;
		} else
			flag = EMPTY;
		Spin_unlock(&lock);
		switch (flag) {
		case SUCCESS:
			break;
		case EMPTY:
			Spin_lock(&empty);
			Spin_lock(&lock);
			item = dequeue();
			Spin_unlock(&lock);
			break;
		case FULL:
			Spin_unlock(&full);
			break;
		}
		while (item-- > 0);	/* consume the data */
	}

	Spin_lock(&lock);
	--nconsumers;
	Spin_unlock(&lock);
}

int
main(void)
{
	if (thread_id() == 0)
		producer();
	else
		consumer();

	return 0;
}

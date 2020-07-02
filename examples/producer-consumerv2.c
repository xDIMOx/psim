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
static int      nfull;
static int      producing[2] = {1, 1};
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
	int             flag;
	int             ct;

	switch(thread_id()) {
	case 0:
		val = 0;
		printhex(producing[0]);
		break;
	case 3:
		val = 1;
		printhex(producing[1]);
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
		} else {
			++nfull;
			flag = FULL;
		}
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
		val += 2;
	}

	Spin_lock(&lock);
	if (thread_id() > 0)
		producing[1] = 0;
	else
		producing[0] = 0;
	Spin_unlock(&lock);

	if (thread_id() > 0)
		return;

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
			if (ct == MAXELEM) {
				--nfull;
				flag = FULL;
			} else
				flag = SUCCESS;
			item = dequeue();
		} else if (!producing[0] && !producing[1]) {
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

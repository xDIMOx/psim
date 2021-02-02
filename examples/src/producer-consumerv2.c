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

#ifndef PRODUCER_WAIT
#define PRODUCER_WAIT 0
#endif

#ifndef CONSUMER_WAIT
#define CONSUMER_WAIT item
#endif

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
	int             tid;
	int             val;
	int             flag;

	tid = thread_id();
	for (val = tid & 1; val < MAXVAL; val += 2) {
		busywait(PRODUCER_WAIT);
		Spin_lock(&lock);
		if (ct < MAXELEM) {
			enqueue(val);
			if (nempty > 0) {
				Spin_unlock(&empty);
				--nempty;
			}
			flag = SUCCESS;
		} else {
			++nfull;
			flag = FULL;
		}
		Spin_unlock(&lock);
		if (flag == FULL) {
			Spin_lock(&full);
		}
	}

	Spin_lock(&lock);

	producing[tid & 1] = 0;

	while (nempty > 0) {
		Spin_unlock(&empty);
		--nempty;
	}

	Spin_unlock(&lock);
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
			item = dequeue();
			if (nfull > 0) {
				Spin_unlock(&full);
				--nfull;
			}
			flag = SUCCESS;
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
			busywait(CONSUMER_WAIT);
			break;
		case EMPTY:
			Spin_lock(&empty);
			break;
		}
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

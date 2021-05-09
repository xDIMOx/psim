/* Check LICENSE file for copyright and license details. */

/*
 * Producer-consumer example
 */

#include "common.h"
#include "semaphore.h"

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

extern int      randuseed;

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
	int             pid;
	int             val;
	int             flag;

	pid = processor_id();
	for (val = pid & 1; val < MAXVAL; val += 2) {
		busywait(PRODUCER_WAIT);
		Sem_P(&lock);
		if (ct < MAXELEM) {
			enqueue(val);
			if (nempty > 0) {
				Sem_V(&empty);
				--nempty;
			}
			flag = SUCCESS;
		} else {
			++nfull;
			flag = FULL;
		}
		Sem_V(&lock);
		if (flag == FULL) {
			Sem_P(&full);
		}
	}

	Sem_P(&lock);

	producing[pid & 1] = 0;

	while (nempty > 0) {
		Sem_V(&empty);
		--nempty;
	}

	Sem_V(&lock);
}

void
consumer(void)
{
	int             item;
	int             flag;

	flag = SUCCESS;
	while (flag != END) {
		Sem_P(&lock);
		if (ct > 0) {
			item = dequeue();
			if (nfull > 0) {
				Sem_V(&full);
				--nfull;
			}
			flag = SUCCESS;
		} else if (!producing[0] && !producing[1]) {
			item = -1;
			flag = END;
		} else {
			++nempty;
			flag = EMPTY;
		}
		Sem_V(&lock);
		switch (flag) {
		case SUCCESS:
			busywait(CONSUMER_WAIT);
			break;
		case EMPTY:
			Sem_P(&empty);
			break;
		}
	}
}

int
main(void)
{
	unsigned int    id;

	id = processor_id();

	/* garantee that randu starts with odd value */
	if (id & 1) {
		randuseed = id * 3;
	} else {
		randuseed = id + 1;
	}

	if (id == 0 || id == 3)
		producer();
	else
		consumer();

	return 0;
}

/* Check LICENSE file for copyright and license details. */

/*
 * Producer-consumer example
 */

#include "comm.h"
#include "common.h"

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

#define NPRODUCERS 2

#ifndef PRODUCER_WAIT
#define PRODUCER_WAIT 0
#endif

#ifndef CONSUMER_WAIT
#define CONSUMER_WAIT item
#endif

#ifndef PRODUCER0
#define PRODUCER0 0		/* producer processor */
#endif

#ifndef PRODUCER1
#define PRODUCER1 2		/* producer processor */
#endif

#define BUFFER 1		/* buffering processor */

static int      ct = 0;
static int      hd;		/* queue's head */
static int      tl;		/* queue's tail */
static int      buf[MAXELEM];

static int      consumers[NCONSUMERS + 1];
static int      producers[NPRODUCERS];

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
buffer(void)
{
	int             data;
	int             ret;
	int             nc;
	int             np;

	np = NPRODUCERS;
	while (np > 0) {
		if (ct < MAXELEM) {
			ret = C2_alt((int *) &producers, NPRODUCERS, &data);
			if (data < 0) {
				--np;
			} else {
				enqueue(data);
			}
		}
		ret = C2_alt((int *) &consumers, NCONSUMERS + 1, &data);
		if (ret >= 0) {
			C2_output(ret, dequeue());
		}
	}

	while (ct > 0) {
		ret = C2_alt((int *) &consumers, NCONSUMERS, &data);
		C2_output(ret, dequeue());
	}

	for (nc = NCONSUMERS; nc > 0; --nc) {
		ret = C2_alt((int *) &consumers, NCONSUMERS, &data);
		C2_output(ret, -1);
	}
}

void
producer(void)
{
	int             i;

	i = (processor_id() == PRODUCER1)? 1 : 0;
	while (i < MAXVAL) {
		busywait(PRODUCER_WAIT);	/* "producing" */
		C2_output(BUFFER, i);
		i += 2;
	}

	C2_output(BUFFER, -1);
}

void
consumer(void)
{
	int             item;

	item = 0;
	while (item >= 0) {
		C2_output(BUFFER, 0);
		item = C2_input(BUFFER);
		busywait(CONSUMER_WAIT);	/* "consuming" */
	}
}

int
main(void)
{
	int             id;
	int             i, c;


	id = processor_id();
	/* garantee that randu starts with odd value */
	if (id & 1) {
		randstate = id * 3;
	} else {
		randstate = id + 1;
	}

	switch (id) {
	case PRODUCER0:
	case PRODUCER1:
		producer();
		break;
	case BUFFER:
		producers[0] = PRODUCER0;
		producers[1] = PRODUCER1;
		i = 0;
		for (c = 0; c < (NCONSUMERS + NPRODUCERS + 1); ++c) {
			if (c == PRODUCER0 || c == PRODUCER1 || c == BUFFER) {
				continue;
			}
			consumers[i++] = c;
		}
		consumers[NCONSUMERS] = -1;	/* default clause */
		buffer();
		break;
	default:
		consumer();
	}

	return 0;
}

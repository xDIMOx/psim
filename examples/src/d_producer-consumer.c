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

#ifndef PRODUCER_WAIT
#define PRODUCER_WAIT 0
#endif

#ifndef CONSUMER_WAIT
#define CONSUMER_WAIT item
#endif

#define PRODUCER 0		/* producer processor */

#define BUFFER 1		/* buffering processor */

static int      ct = 0;
static int      hd;		/* queue's head */
static int      tl;		/* queue's tail */
static int      buf[MAXELEM];

static int      consumers[NCONSUMERS + 1];

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

	for (;;) {
		if (ct < MAXELEM) {
			data = C2_input(PRODUCER);
			if (data < 0) {
				goto REMITEMS;
			} else {
				enqueue(data);
			}
		}
		ret = C2_alt((int *) &consumers, NCONSUMERS + 1, &data);
		if (ret >= 0) {
			C2_output(ret, dequeue());
		}
	}

REMITEMS:
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

	for (i = 0; i < MAXVAL; ++i) {
		busywait(PRODUCER_WAIT);	/* "producing" */
		C2_output(BUFFER, i);
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
		if (item >= 0) {
			busywait(CONSUMER_WAIT);	/* "consuming" */
		}
	}
}

int
main(void)
{
	int             i, c;

	switch (thread_id()) {
	case PRODUCER:
		producer();
		break;
	case BUFFER:
		for (i = 0, c = BUFFER + 1; i < NCONSUMERS; ++i, ++c) {
			consumers[i] = c;
		}
		consumers[NCONSUMERS] = -1;	/* default clause */
		buffer();
		break;
	default:
		consumer();
	}

	return 0;
}

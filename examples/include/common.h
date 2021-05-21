/* Check LICENSE file for copyright and license details. */

#ifndef rand
#define rand xorshift32
#endif

long            randstate;

/*
 * Common functions
 */

int             processor_id(void);
void            putchar(char ch);
void            printhex(int w);
void            busywait(int nloops);
long            randu(void);
long            xorshift32(void);
int             rem(int x, int y);

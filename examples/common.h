/* Check LICENSE file for copyright and license details. */

/*
 * Common functions
 */

int             thread_id(void);
void            putchar(char ch);
void            printhex(int w);
void            tog_lockperf0(void);
void            tog_lockperf1(void);
void            tog_lockperf2(void);
void            tog_lockperf3(void);
void            tog_lockperf4(void);
void            tog_lockperf5(void);
void            busywait(int nloops);
long            randu(void);
int             rem(int x, int y);

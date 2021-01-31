/* Check LICENSE file for copyright and license details. */

/*
 * Hello world
 */

#include <common.h>

int
main(void)
{
	int             i;

	char           *str;

	str = "hello, world\n";
	for (i = 0; str[i]; ++i) {
		putchar(str[i]);
	}

	return 0;
}

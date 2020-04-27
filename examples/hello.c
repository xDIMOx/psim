/* Check LICENSE file for copyright and license details. */

/*
 * Hello world
 */

int
main(void)
{
	int             i;

	char           *str;

	volatile char  *stdout;

	stdout = (char *) 0xFFFF;

	str = "hello, world\n";
	for (i = 0; str[i]; ++i)
		*stdout = str[i];

	return 0;
}

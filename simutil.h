/* Check LICENSE file for copyright and license details. */

/*
 * Data structures for the simulation
 */

enum Org {
	NONE,
	SHRMEM,
	NET,
};

struct ProgInfo {
	off_t           size;
	size_t          entry;
	char           *name;
	unsigned char  *elf;
};

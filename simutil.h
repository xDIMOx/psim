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
	Elf32_Addr      entry;
	char           *name;
	unsigned char  *elf;
};

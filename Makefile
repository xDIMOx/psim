# Check LICENSE file for copyright and license details.

.POSIX:

include config.mk

SRC = main.c cpu.c mem.c datapath.c
OBJ = ${SRC:.c=.o}

all: psim

.c.o:
	@echo CC -c $<
	@${CC} ${CFLAGS} -c $<

main.o: main.c cpu.h mem.h datapath.h
cpu.o: cpu.c cpu.h
mem.o: mem.c mem.h
datapath.o: datapath.c datapath.h cpu.h mem.h instr.h

psim: ${OBJ}
	@echo CC -o $@
	@${CC} ${OBJ} ${LDFLAGS} -o $@

clean:
	@echo cleaning
	@rm -f ${OBJ} psim

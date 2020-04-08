# Check LICENSE file for copyright and license details.

.POSIX:

include config.mk

SRC = main.c cpu.c
OBJ = ${SRC:.c=.o}

all: psim

.c.o:
	@echo CC -c $<
	@${CC} ${CFLAGS} -c $<

cpu.o: cpu.c cpu.h

psim: ${OBJ}
	@echo CC -o $@
	@${CC} ${OBJ} ${LDFLAGS} -o $@

clean:
	@echo cleaning
	@rm -f ${OBJ} psim

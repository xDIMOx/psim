# Check LICENSE file for copyright and license details.

.POSIX:

include config.mk

SRC = main.c
OBJ = ${SRC:.c=.o}

all: psim

.c.o:
	@echo CC -c $<
	@${CC} ${CFLAGS} -c $<

psim: ${OBJ}
	@echo CC -o $@
	@${CC} ${OBJ} ${LDFLAGS} -o $@

clean:
	@echo cleaning
	@rm -f ${OBJ} psim

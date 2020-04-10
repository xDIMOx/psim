# Check LICENSE file for copyright and license details.

.POSIX:

include config.mk

# Assembly source files
ASRC = start.s

# C source files
CSRC =

# Object files
OBJ = ${ASRC:.s=.o} ${CSRC:.c=.o}

# executables
EXE = ${CSRC:.c=}

all: ${EXE}

.s.o:
	@echo AS $<
	@${AS} ${AFLAGS} -o $@ $<

.c.o:
	@echo CC -c $<
	@${CC} ${CFLAGS} -c $<

start.o: start.s

clean:
	@echo cleaning
	@rm -f ${OBJ} ${EXE}

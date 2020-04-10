# Check LICENSE file for copyright and license details.

.POSIX:

include config.mk

# Assembly source files
ASRC = start.s

# C source files
CSRC = null.c

# Object files
OBJ = ${ASRC:.s=.o} ${CSRC:.c=.o}

# executables
EXE = ${CSRC:.c=}

# prerequisites
NULL_PREREQ = start.o null.o

all: ${EXE}

.s.o:
	@echo AS $<
	@${AS} ${AFLAGS} -o $@ $<

.c.o:
	@echo CC -c $<
	@${CC} ${CFLAGS} -c $<

start.o: start.s
null.o: null.c

null: ${NULL_PREREQ}
	@echo LD -o $@
	@${LD} ${LDFLAGS} ${NULL_PREREQ} -o $@

clean:
	@echo cleaning
	@rm -f ${OBJ} ${EXE}

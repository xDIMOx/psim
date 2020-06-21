# Check LICENSE file for copyright and license details.

.POSIX:

include config.mk

# Assembly source files
ASRC = start.s common.s spinlock.s

# C source files
CSRC = null.c hello.c producer-consumer.c

# Object files
OBJ = ${ASRC:.s=.o} ${CSRC:.c=.o}

# executables
EXE = ${CSRC:.c=}

# prerequisites
NULL_PREREQ  = start.o null.o
HELLO_PREREQ = start.o hello.o
PRODCON_PREREQ = start.o common.o spinlock.o producer-consumer.o
PRODCONV2_PREREQ = start.o common.o spinlock.o producer-consumerv2.o

all: ${EXE}

.s.o:
	@echo AS $<
	@${AS} ${AFLAGS} -o $@ $<

.c.o:
	@echo CC -c $<
	@${CC} ${CFLAGS} -c $<

start.o: start.s
common.o: common.s
spinlock.o: spinlock.s
null.o: null.c
hello.o: hello.c

producer-consumer.o: producer-consumer.c
	@echo CC -c $<
	@${CC} ${CFLAGS} ${PRODCON_CFLAGS} -c $<

producer-consumerv2.o: producer-consumerv2.c
	@echo CC -c $<
	@${CC} ${CFLAGS} ${PRODCONV2_CFLAGS} -c $<

null: ${NULL_PREREQ}
	@echo LD -o $@
	@${LD} ${LDFLAGS} ${NULL_PREREQ} -o $@

hello: ${HELLO_PREREQ}
	@echo LD -o $@
	@${LD} ${LDFLAGS} ${HELLO_PREREQ} -o $@

producer-consumer: ${PRODCON_PREREQ}
	@echo LD -o $@
	@${LD} ${LDFLAGS} ${PRODCON_PREREQ} -o $@

producer-consumerv2: ${PRODCONV2_PREREQ}
	@echo LD -o $@
	@${LD} ${LDFLAGS} ${PRODCONV2_PREREQ} -o $@

clean:
	@echo cleaning
	@rm -f ${OBJ} ${EXE}

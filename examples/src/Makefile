# Check LICENSE file for copyright and license details.

.POSIX:

include config.mk

LIBCOMMON_SRC = libcommon/busywait.s libcommon/printhex.s libcommon/putchar.s \
                libcommon/randu.s libcommon/rem.s libcommon/processor_id.s
LIBCOMMON_OBJ = ${LIBCOMMON_SRC:.s=.o} 

LIBSEM_SRC = libsem/Sem_P.s libsem/Sem_V.s
LIBSEM_OBJ = ${LIBSEM_SRC:.s=.o}

LIBCOMM_SRC = libcomm/C2_alt.s libcomm/C2_input.s libcomm/C2_output.s
LIBCOMM_OBJ = ${LIBCOMM_SRC:.s=.o}

LIBS = libcommon.a libsem.a libcomm.a

SRC = null.c hello.c producer-consumer.c producer-consumerv2.c \
      dining-philosophers.c d_producer-consumer.c d_producer-consumerv2.c \
      d_dining-philosophers.c
EXE = ${SRC:.c=}

OBJ = ${SRC:.c=.o} start.o ${LIBCOMMON_OBJ} ${LIBSEM_OBJ} ${LIBCOMM_OBJ}

# prerequisites
NULL_PREREQ = start.o null.o
HELLO_PREREQ = start.o hello.o libcommon.a
PRODCON_PREREQ = start.o producer-consumer.o libcommon.a libsem.a
PRODCONV2_PREREQ = start.o producer-consumerv2.o libcommon.a libsem.a
DININGPHIL_PREREQ = start.o dining-philosophers.o libcommon.a libsem.a
DPRODCON_PREREQ = start.o d_producer-consumer.o libcommon.a libcomm.a
DPRODCONV2_PREREQ = start.o d_producer-consumerv2.o libcommon.a libcomm.a
DDININGPHIL_PREREQ = start.o d_dining-philosophers.o libcommon.a libcomm.a

all: ${EXE}

.s.o:
	@echo AS $<
	@${AS} ${AFLAGS} -o $@ $<

.c.o:
	@echo CC -c $<
	@${CC} ${CFLAGS} -c $<

start.o: start.s

null.o: null.c

hello.o: hello.c

producer-consumer.o: producer-consumer.c
	@echo CC -c $<
	@${CC} ${CFLAGS} ${PRODCON_CFLAGS} -c $<

producer-consumerv2.o: producer-consumerv2.c
	@echo CC -c $<
	@${CC} ${CFLAGS} ${PRODCONV2_CFLAGS} -c $<

dining-philosophers.o: dining-philosophers.c
	@echo CC -c $<
	@${CC} ${CFLAGS} ${DINPHIL_CFLAGS} -c $<

d_producer-consumer.o: d_producer-consumer.c
	@echo CC -c $<
	@${CC} ${CFLAGS} ${DPRODCON_CFLAGS} -c $<

d_producer-consumerv2.o: d_producer-consumerv2.c
	@echo CC -c $<
	@${CC} ${CFLAGS} ${DPRODCONV2_CFLAGS} -c $<

d_dining-philosophers.o: d_dining-philosophers.c
	@echo CC -c $<
	@${CC} ${CFLAGS} ${DDINPHIL_CFLAGS} -c $<

libcommon.a: ${LIBCOMMON_OBJ}
	@echo AR libcommon.a
	@${AR} -rcs $@ ${LIBCOMMON_OBJ}

libsem.a: ${LIBSEM_OBJ}
	@echo AR libcspin.a
	@${AR} -rcs $@ ${LIBSEM_OBJ}

libcomm.a: ${LIBCOMM_OBJ}
	@echo AR libcomm.a
	@${AR} -rcs $@ ${LIBCOMM_OBJ}

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

dining-philosophers: ${DININGPHIL_PREREQ}
	@echo LD -o $@
	@${LD} ${LDFLAGS} ${DININGPHIL_PREREQ} -o $@

d_producer-consumer: ${DPRODCON_PREREQ}
	@echo LD -o $@
	@${LD} ${LDFLAGS} ${DPRODCON_PREREQ} -o $@

d_producer-consumerv2: ${DPRODCONV2_PREREQ}
	@echo LD -o $@
	@${LD} ${LDFLAGS} ${DPRODCONV2_PREREQ} -o $@

d_dining-philosophers: ${DDININGPHIL_PREREQ}
	@echo LD -o $@
	@${LD} ${LDFLAGS} ${DDININGPHIL_PREREQ} -o $@

clean:
	@echo cleaning
	@rm -f ${EXE} ${OBJ} ${LIBS}

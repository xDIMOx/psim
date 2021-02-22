# Check LICENSE file for copyright and license details.

# Customize for your needs bellow

AS     = mipsel-elf-as
AFLAGS = -Os -mips32r2 -msoft-float -I../include/

CC     = mipsel-elf-gcc
CFLAGS = -O1 -nostartfiles -mips32r2 -msoft-float -fno-builtin -mno-memcpy \
         -fno-stack-protector -fno-delayed-branch -mno-gpopt -I../include/

LD      = mipsel-elf-ld
LDFLAGS = -static

# definitions for producer-consumer program
PRODCON_CFLAGS = -DNCONSUMERS=1 -DMAXELEM=16 -DMAXVAL=1024 -DPRODUCER_WAIT=0 \
                 -DCONSUMER_WAIT=item

# definitions for producer-consumer (v2) program
PRODCONV2_CFLAGS = -DNCONSUMERS=2 -DMAXELEM=16 -DMAXVAL=1024 \
                   -DPRODUCER_WAIT=0 -DCONSUMER_WAIT=item

# definitions for dining philosophers program
DINPHIL_CFLAGS = -DIDEAS=10 -DPHILOS=5 -DTHINK=1 -DEAT=1

# definitions for d_producer-consumer program
DPRODCON_CFLAGS = -DNCONSUMERS=1 -DMAXELEM=16 -DMAXVAL=1024 -DPRODUCER_WAIT=0 \
                  -DCONSUMER_WAIT=item

# definitions for producer-consumer (v2) program
DPRODCONV2_CFLAGS = -DNCONSUMERS=1 -DMAXELEM=16 -DMAXVAL=1024 \
                   -DPRODUCER_WAIT=0 -DCONSUMER_WAIT=item -DPRODUCER0=0 \
                   -DPRODUCER1=2 -DBUFFER=1

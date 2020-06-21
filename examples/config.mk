# Check LICENSE file for copyright and license details.

# Customize for your needs bellow

AS     = mipsel-elf-as
AFLAGS = -Os -mips32r2 -msoft-float

CC     = mipsel-elf-gcc
CFLAGS = -O1 -nostartfiles -mips32r2 -msoft-float -fno-builtin -mno-memcpy \
         -fno-stack-protector -fno-delayed-branch -mno-gpopt

LD      = mipsel-elf-ld
LDFLAGS = -static

# definitions for producer-consumer program
PRODCON_CFLAGS = -DNCONSUMERS=1 -DMAXELEM=16 -DMAXVAL=1024

# definitions for producer-consumer (v2) program
PRODCONV2_CFLAGS = -DNCONSUMERS=2 -DMAXELEM=16 -DMAXVAL=1024

# Check LICENSE file for copyright and license details.

# Customize for your needs bellow

AS     = mipsel-elf-as
AFLAGS = -Os -mips32r2 -msoft-float

CC     = mipsel-elf-gcc
CFLAGS = -O1 -nostartfiles -mips32r2 -msoft-float -fno-builtin -mno-memcpy \
         -fno-stack-protector -fno-delayed-branch -mno-gpopt

LD      = mipsel-elf-ld
LDFLAGS = -static

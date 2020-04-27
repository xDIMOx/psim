# Check LICENSE file for copyright and license details.

# Customize for your needs bellow

AS     = mipsel-elf-as
AFLAGS = -Os -mips32r2 -msoft-float

CC     = mipsel-elf-gcc
CFLAGS = -Os -nostartfiles -mips32r2 -msoft-float -fno-builtin -mno-memcpy \
         -fno-stack-protector -fno-delayed-branch

LD      = mipsel-elf-ld
LDFLAGS = -static

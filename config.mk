# Check LICENSE file for copyright and license details.

CC = gcc

CFLAGS  = -std=c99 -Wall -Wpedantic -pedantic-errors -Wextra -Wfatal-errors \
          -Wformat=2 -Wlogical-op -Wjump-misses-init -Wshadow \
          -Wpadded -Wredundant-decls -Wcast-qual -Wcast-align \
          -Wstrict-overflow=5 -Wno-redundant-decls -fno-builtin \
          -Os -D_XOPEN_SOURCE=700
LDFLAGS = -s -static -lpthread

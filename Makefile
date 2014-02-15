GCC=gcc
CLANG=clang
CC=$(GCC)

CFLAGS=-std=c99 -pedantic -Wall -O2

.PHONY: clean

all:datluj
clean:
	-rm datluj.clang datluj

datluj: datluj.c
	$(GCC)   $(CFLAGS) $? -o $@
	$(CLANG) $(CFLAGS) $? -o $@.clang

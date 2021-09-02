SOURCES := $(wildcard *.c)
BINARIES := $(patsubst t%.c,a%.out,${SOURCES})

all: ${BINARIES}

a.out: t.c
	cc -m32 $? -o $@
# Plik makefile dla projektu 1 z SO2 - problem jedzących filozofów

CC=gcc
CFLAGS=-Wall -Iinclude/ -pthread

SRCDIR=./src/
OUTDIR=./out/

build: $(SRCDIR)main.c
	mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) -o $(OUTDIR)main $(SRCDIR)main.c

clean: $(OUTDIR)
	rm -r $(OUTDIR)

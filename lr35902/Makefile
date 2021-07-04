CC=gcc
CFLAGS=-g -Werror -pedantic
GCOV=-fprofile-arcs -ftest-coverage
OUTDIR=build/

.PHONY: default clean
default: $(OUTDIR)/lr35902.o

clean:
	rm -rf build/

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/lr35902.o: $(OUTDIR) lr35902.c
	$(CC) -DLR35902_BUILTIN_INTERRUTS -c $(CFLAGS) lr35902.c -o $@

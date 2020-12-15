CC=clang
CFLAGS=-g -Werror -pedantic
ALL_MACHINES = stoc-6502 stoc-2a03 stoc-6510 
BUILD_DIR := build/
SOURCES = tests.c labels.c asm65.c reg.c main.c instr.c stoc.c search.c exh.c
GENERATED = gen-6502.c gen-6510.c gen-65c02.c gen-2a03.c
GENOBJECTS = $(GENERATED:%.c=$(BUILD_DIR)%.o)
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)%.o)

.PHONY: default
default: $(ALL_MACHINES)
	grep TODO *.c || true
	grep FIXME *.c || true

$(GENOBJECTS): $(BUILD_DIR)%.o:%.c
	mkdir -p build/
	$(CC) -c $(CFLAGS) -o $@ $^

$(OBJECTS): $(BUILD_DIR)%.o:%.c
	mkdir -p build/
	$(CC) -c $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -rf gen-*.c *.o $(ALL_MACHINES)
	rm -r build
	make -C fake6502/ clean

# Source file per target, generated at compile-time
gen-6502.c gen-2a03.c: generate.py
	cat opcodes/basic-6502 | ./generate.py > $@

gen-6510.c: generate.py
	cat opcodes/basic-6502 opcodes/nmos-6502-extra | ./generate.py > $@

gen-65c02.c: generate.py
	cat opcodes/basic-6502 opcodes/cmos-6502-extra | ./generate.py > $@

.PHONY: fake6502_build
fake6502_build:

# The executables
stoc-6502: $(BUILD_DIR)gen-6502.o $(OBJECTS)
	make -C fake6502/ fake6502.o
	$(CC) $(CFLAGS) -o $@ $^ fake6502/fake6502.o

stoc-6510: $(BUILD_DIR)gen-6510.o $(OBJECTS)
	make -C fake6502/ fake6502.o
	$(CC) $(CFLAGS) -o $@ $^ fake6502/fake6502.o

stoc-2a03: $(BUILD_DIR)gen-2a03.o $(OBJECTS)
	make -C fake6502/ fake2a03.o
	$(CC) $(CFLAGS) -o $@ $^ fake6502/fake2a03.o

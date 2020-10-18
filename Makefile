CC=clang
CFLAGS=-g -Werror -pedantic
ALL_MACHINES = stoc-6502 stoc-2a03 stoc-6510 
BUILD_DIR := build/
SOURCES = tests.c exh.c labels.c asm65.c reg.c main.c instr.c
GENERATED = gen-6502.c gen-6510.c gen-65c02.c gen-2a03.c
GENOBJECTS = $(GENERATED:%.c=$(BUILD_DIR)%.o)
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)%.o)

.PHONY: default
default: $(ALL_MACHINES)
	grep TODO *.c || true
	grep FIXME *.c || true

$(GENOBJECTS): $(BUILD_DIR)%.o:%.c
	$(CC) -c $(CFLAGS) -o $@ $^

$(OBJECTS): $(BUILD_DIR)%.o:%.c
	$(CC) -c $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -rf gen-*.c *.o $(ALL_MACHINES)
	rm -r build

# Emulators to run code-sequences on.
$(BUILD_DIR)emu-6502.o: emulators/fake6502.c emulators/fake6502.h
	mkdir -p $(BUILD_DIR)
	$(CC) -c $(CFLAGS) emulators/fake6502.c -o $@

$(BUILD_DIR)emu-2a03.o: emulators/fake6502.c emulators/fake6502.h
	mkdir -p $(BUILD_DIR)
	$(CC) -c $(CFLAGS) emulators/fake6502.c -D NESCPU -o $@

$(BUILD_DIR)emu-6510.o: emulators/fake6502.c emulators/fake6502.h
	mkdir -p $(BUILD_DIR)
	$(CC) -c $(CFLAGS) emulators/fake6502.c -D UNDOCUMENTED -o $@

# Source file per target, generated at compile-time
gen-6502.c gen-2a03.c: generate.py
	cat opcodes/basic-6502 | ./generate.py > $@

gen-6510.c: generate.py
	cat opcodes/basic-6502 opcodes/nmos-6502-extra | ./generate.py > $@

gen-65c02.c: generate.py
	cat opcodes/basic-6502 opcodes/cmos-6502-extra | ./generate.py > $@

# The executables
stoc-6502: $(BUILD_DIR)emu-6502.o $(BUILD_DIR)gen-6502.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

stoc-6510: $(BUILD_DIR)emu-6510.o $(BUILD_DIR)gen-6510.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

stoc-2a03: $(BUILD_DIR)emu-2a03.o $(BUILD_DIR)gen-2a03.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

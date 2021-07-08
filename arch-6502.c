#include "decl.h"
#include "fake6502/fake6502.h"
#include "stoc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int opcode_length(uint8_t op); // from generate.py

void arch_init(stoc_t *c) {
    c->emu = malloc(sizeof(context_t));
    memset(c->emu, 0, sizeof(context_t)); // shut valgrind up a bit
}

void arch_deinit(stoc_t *c) { free(c->emu); }

uint8_t mem_read(context_t *c, uint16_t address) { return c->mem[address]; }

uint8_t memory_read(stoc_t *c, uint16_t address) {
    return mem_read(c->emu, address);
}

void mem_write(context_t *c, uint16_t address, uint8_t val) {
    c->mem[address] = val;
}

void memory_write(stoc_t *c, uint16_t address, uint8_t val) {
    mem_write(c->emu, address, val);
}

int live_in_a(stoc_t *c, decl_t *d, uint8_t **scram) {
    ((context_t *)c->emu)->a = consume_scram(scram);
    return 0;
}

int live_in_x(stoc_t *c, decl_t *d, uint8_t **scram) {
    ((context_t *)c->emu)->x = consume_scram(scram);
    return 0;
}

int live_in_y(stoc_t *c, decl_t *d, uint8_t **scram) {
    ((context_t *)c->emu)->y = consume_scram(scram);
    return 0;
}

int live_out_a(stoc_t *c, decl_t *d, uint8_t **scram) {
    uint8_t s = consume_scram(scram);
    if (((context_t *)c->emu)->a == s) {
        return 0;
    } else {
        return 1;
    }
}

int setup_live_out_a(stoc_t *c, decl_t *d, uint8_t **scram) {
    output_scram(scram, ((context_t *)c->emu)->a);
    return 0;
}

int live_out_x(stoc_t *c, decl_t *d, uint8_t **scram) {
    return ((context_t *)c->emu)->x != consume_scram(scram);
}

int setup_live_out_x(stoc_t *c, decl_t *d, uint8_t **scram) {
    output_scram(scram, ((context_t *)c->emu)->x);
    return 0;
}

int live_out_y(stoc_t *c, decl_t *d, uint8_t **scram) {
    return ((context_t *)c->emu)->y != consume_scram(scram);
}

int setup_live_out_y(stoc_t *c, decl_t *d, uint8_t **scram) {
    output_scram(scram, ((context_t *)c->emu)->y);
    return 0;
}

int live_in_stack(stoc_t *c, decl_t *d, uint8_t **scram) {
    memory_write(c, 0x0100 + ((context_t *)c->emu)->s--, consume_scram(scram));
    return 0;
}

int setup_live_in_stack(stoc_t *c, decl_t *d, uint8_t **scram) {
    **scram = rand();
    return live_in_stack(c, d, scram);
}

int live_out_stack(stoc_t *c, decl_t *d, uint8_t **scram) {
    return consume_scram(scram) !=
           memory_read(c, 0x0100 + ((context_t *)c->emu)->s--);
}

int setup_live_out_stack(stoc_t *c, decl_t *d, uint8_t **scram) {
    output_scram(scram, memory_read(c, 0x0100 + ((context_t *)c->emu)->s--));
    return 0;
}

int start_decl(stoc_t *c, decl_t *d, uint8_t **scram) {
    ((context_t *)c->emu)->a = consume_scram(scram);
    ((context_t *)c->emu)->x = consume_scram(scram);
    ((context_t *)c->emu)->y = consume_scram(scram);
    ((context_t *)c->emu)->s = consume_scram(scram);
    ((context_t *)c->emu)->flags = consume_scram(scram);
    return 0;
}

int setup_start_decl(stoc_t *c, decl_t *d, uint8_t **scram) {
    (*scram)[0] = rand();
    (*scram)[1] = rand();
    (*scram)[2] = rand();
    (*scram)[3] = rand();
    (*scram)[4] = rand();
    return start_decl(c, d, scram);
}

#define COMPUTEBUDGET 10000
int run_decl(stoc_t *c, decl_t *d, uint8_t **scram) {
    addr_t org = c->program.org;
    context_t *fake6502 = c->emu;
    fake6502->pc = org;
    fake6502->clockticks = 0;
    for (int i = 0; i < COMPUTEBUDGET; i++) {
        if (fake6502->pc == c->program.end) {
            c->clockticks = fake6502->clockticks;
            return false; // the routine exited normally
        }
        if (fake6502->pc < org)
            return true; // program counter out of bounds
        if (fake6502->pc > c->program.end)
            return true; // program counter out of bounds
        step(fake6502);
    }
    return true; // took too long
}

void register_in_name(decl_t *d, char *name) {
    if (!strcmp(name, "a")) {
        d->fn = live_in_a;
        return;
    }
    if (!strcmp(name, "x")) {
        d->fn = live_in_x;
        return;
    }
    if (!strcmp(name, "y")) {
        d->fn = live_in_y;
        return;
    }
    fprintf(stderr, "Unknown register name \"%s\" in " __FILE__ "\n", name);
    exit(1);
}

void register_out_name(decl_t *d, char *name) {
    if (!strcmp(name, "a")) {
        d->fn = live_out_a;
        d->setup = setup_live_out_a;
        return;
    }
    if (!strcmp(name, "x")) {
        d->fn = live_out_x;
        d->setup = setup_live_out_x;
        return;
    }
    if (!strcmp(name, "y")) {
        d->fn = live_out_y;
        d->setup = setup_live_out_y;
        return;
    }
    fprintf(stderr, "Unknown register name \"%s\" in " __FILE__ "\n", name);
    exit(1);
}

void hexdump(stoc_t *c) {
    rewrite_t *r = &c->program;
    printf("; starting at $%04x\n", r->org);
    printf("; %d instructions\n", r->length);
    printf("; %d bytes\n; %d clockticks\n", r->blength, c->clockticks);
    for (int i = 0; i < r->length; i++) {
        uint8_t instr = r->instructions[i].opcode;
        if (is_implied_instruction(instr)) {
            fprintf(stderr, "\t%s\n", opnames[instr]);
        } else if (is_immediate_instruction(instr)) {
            fprintf(stderr, "\t%s #$%02x\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else if (is_zero_page_instruction(instr)) {
            fprintf(stderr, "\t%s $%02x\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else if (is_zero_page_x_instruction(instr)) {
            fprintf(stderr, "\t%s $%02x,x\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else if (is_zero_page_y_instruction(instr)) {
            fprintf(stderr, "\t%s $%02x,y\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else if (is_absolute_instruction(instr)) {
            fprintf(stderr, "\t%s $%04x\n", opnames[instr],
                    r->instructions[i].operand);
        } else if (is_absolute_y_instruction(instr)) {
            fprintf(stderr, "\t%s $%04x,y\n", opnames[instr],
                    r->instructions[i].operand);
        } else if (is_absolute_x_instruction(instr)) {
            fprintf(stderr, "\t%s $%04x,x\n", opnames[instr],
                    r->instructions[i].operand);
        } else if (is_indirect_instruction(instr)) {
            fprintf(stderr, "\t%s $(%04x)\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else if (is_indirect_x_instruction(instr)) {
            fprintf(stderr, "\t%s $(%02x),x\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else if (is_relative_instruction(instr)) {
            fprintf(stderr, "\t%s * + %d\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else if (is_indirect_y_instruction(instr)) {
            fprintf(stderr, "\t%s $(%02x,y)\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else {
            fprintf(stderr, "\t$%02x", instr);
            if (opcode_length(instr) > 1)
                fprintf(stderr, " $%02x", r->instructions[i].operand & 0x00ff);
            if (opcode_length(instr) > 2)
                fprintf(stderr, " $%02x", r->instructions[i].operand >> 8);
            fprintf(stderr, "\n");
        }
    }
    fprintf(stderr, "\n");
}

static pick_t zp_addresses;

void randomise_opcode(instruction_t *i) {
retry:
    i->opcode = rand() & 0xff;
    if (!opcode_legal_p(i->opcode))
        goto retry;
    if (!addressing_modes[i->opcode])
        goto retry;
    if (addressing_modes[i->opcode] == &mode_relative)
        goto retry;
}

bool randomise_operand(rewrite_t *p, instruction_t *i) {
    // TODO: Can we think of a more sensible solution here?
    pick_t *mode = addressing_modes[i->opcode];
    if (!mode)
        return false;

    if (mode == &mode_implied)
        return true;

    if (mode == &mode_immediate)
        return random_constant(&i->operand);

    if (mode == &mode_indirect_x)
        return pick_at_random(&zp_addresses, &i->operand);

    if (mode == &mode_indirect_y)
        return pick_at_random(&zp_addresses, &i->operand);

    if (mode == &mode_zero_page)
        return pick_at_random(&zp_addresses, &i->operand);

    if (mode == &mode_zero_page_x)
        return pick_at_random(&zp_addresses, &i->operand);

    if (mode == &mode_zero_page_y)
        return pick_at_random(&zp_addresses, &i->operand);

    if (mode == &mode_indirect)
        return random_address(&i->operand);

    if (mode == &mode_absolute)
        return random_address(&i->operand);

    if (mode == &mode_absolute_x)
        return random_address(&i->operand);

    if (mode == &mode_absolute_y)
        return random_address(&i->operand);

    if (mode == &mode_relative) {
        printf("relative instruction error\n");
        exit(1);
    }

    printf("unknown mode for %02x error\n", i->opcode);
    exit(1);
}

void mutate_opcode(instruction_t *i) {
    uint16_t opcode = i->opcode;
    pick_t *m = addressing_modes[opcode];
    pick_at_random(m, &opcode);
    i->opcode = opcode;
}

void archsearch_init() {
    // We're going to find all defined zero-page addresses and add them to the
    // zp_addresses pick.
    initialize_pick(&zp_addresses);

    iterator_t addr_it;
    uint16_t address;

    for (iterator_init(&addresses, &addr_it); pick_iterate(&addr_it, &address);)
        if (address < 256)
            pick_insert(&zp_addresses, address);
}

bool exhsearch(stoc_t *reference, stoc_t *rewrite,  bool (*continuation)(stoc_t * reference, stoc_t * rewrite), int i) {
    if (i < 0) {
        // End of this branch
        if (continuation(reference, rewrite)) {
			return true;
        }
        return false;
    }

    iterator_t opciter;
    iterator_t operiter;

    uint16_t opcode;

    for (iterator_init(&mode_implied, &opciter);
         pick_iterate(&opciter, &opcode);) {
        rewrite->program.instructions[i].opcode = opcode;
        if (exhsearch(reference, rewrite, continuation, i - 1))
            return true;
    }

    for (iterator_init(&mode_immediate, &opciter);
         pick_iterate(&opciter, &opcode);) {
        rewrite->program.instructions[i].opcode = opcode;
        for (iterator_init(&constants, &operiter); pick_iterate(
                 &operiter, &rewrite->program.instructions[i].operand);)
            if (exhsearch(reference, rewrite, continuation, i - 1))
                return true;
    }

    // TODO: Add other kinds of instructions here.
    return false;
}

void install(stoc_t *c) {
    rewrite_t *r = &c->program;
    uint16_t addr = r->org;
    for (int i = 0; i < r->length; i++) {
        uint8_t instr = r->instructions[i].opcode & 0xff;
        r->instructions[i].address = addr;
        memory_write(c, addr++, instr);
        if (opcode_length(instr) > 1)
            memory_write(c, addr++, r->instructions[i].operand & 0x00ff);
        if (opcode_length(instr) > 2)
            memory_write(c, addr++, r->instructions[i].operand >> 8);
    }
    r->blength = addr - r->org;
    r->end = r->org + r->blength;
}

void read_prog(rewrite_t * r, uint8_t * raw, int length) {
	int offs = 0;
	int ins = 0;
	int address = r->org;

	for(;;) {
		if(offs > length) break;

		instruction_t *i = &(r->instructions[ins++]);


		i->opcode = raw[offs++];
		i->address = address;

		if (opcode_length(i->opcode) == 2) {
			i->operand = raw[offs++];
		}

		if (opcode_length(i->opcode) == 3) {
			uint16_t l = raw[offs++];
			uint16_t h = raw[offs++];
			i->operand = (h << 8) | l;
		}
	}
	r->length = ins;
}

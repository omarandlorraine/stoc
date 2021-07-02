#include "decl.h"
#include "stoc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t mem_read(stoc_t *c, uint16_t address) { return c->mem[address]; }

void mem_write(stoc_t *c, uint16_t address, uint8_t val) {
    c->mem[address] = val;
}

int live_in_a(stoc_t *c, decl_t *d, uint8_t **scram) {
    c->a = consume_scram(scram);
    return 0;
}

int live_in_x(stoc_t *c, decl_t *d, uint8_t **scram) {
    c->x = consume_scram(scram);
    return 0;
}

int live_in_y(stoc_t *c, decl_t *d, uint8_t **scram) {
    c->x = consume_scram(scram);
    return 0;
}

int live_out_a(stoc_t *c, decl_t *d, uint8_t **scram) {
    uint8_t s = consume_scram(scram);
    if (c->a == s) {
        return 0;
    } else {
        return 1;
    }
}

int setup_live_out_a(stoc_t *c, decl_t *d, uint8_t **scram) {
    output_scram(scram, c->a);
    return 0;
}

int live_out_x(stoc_t *c, decl_t *d, uint8_t **scram) {
    return c->x != consume_scram(scram);
}

int setup_live_out_x(stoc_t *c, decl_t *d, uint8_t **scram) {
    output_scram(scram, c->x);
    return 0;
}

int live_out_y(stoc_t *c, decl_t *d, uint8_t **scram) {
    return c->y != consume_scram(scram);
}

int setup_live_out_y(stoc_t *c, decl_t *d, uint8_t **scram) {
    output_scram(scram, c->y);
    return 0;
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
    printf("; %d bytes\n; %lld clockticks\n", r->blength, c->clockticks);
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

void randomise_opcode(instruction_t * i) {
retry:
    i->opcode = rand();
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

void install(stoc_t *c) {
    rewrite_t *r = &c->program;
    uint16_t addr = r->org;
    for (int i = 0; i < r->length; i++) {
        uint8_t instr = r->instructions[i].opcode;
        r->instructions[i].address = addr;
        mem_write(c, addr++, instr);
        if (opcode_length(instr) > 1)
            mem_write(c, addr++, r->instructions[i].operand & 0x00ff);
        if (opcode_length(instr) > 2)
            mem_write(c, addr++, r->instructions[i].operand >> 8);
    }
    r->blength = addr - r->org;
    r->end = r->org + r->blength;
}


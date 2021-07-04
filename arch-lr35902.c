#include "arch-lr35902.h"
#include "decl.h"
#include "lr35902/lr35902.h"
#include "stoc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ADDRSPACE 65536

void arch_init(stoc_t *c) {
    c->emu = malloc(sizeof(struct LR35902));
    ((struct LR35902 *)c->emu)->userdata = malloc(ADDRSPACE);
    memset(c->emu, 0, sizeof(struct LR35902)); // shut valgrind up a bit
    memset(((struct LR35902 *)c->emu)->userdata, 0, ADDRSPACE);
}

void arch_deinit(stoc_t *c) {
    free(((struct LR35902 *)c->emu)->userdata);
    free(c->emu);
}

unsigned char LR35902_read(void *user, unsigned short addr) {
    return ((char *)user)[addr];
}

uint8_t memory_read(stoc_t *c, uint16_t address) {
    return LR35902_read(((struct LR35902 *)c->emu)->userdata, address);
}

void LR35902_write(void *user, unsigned short addr, unsigned char value) {
    ((char *)user)[addr] = value;
}

void memory_write(stoc_t *c, uint16_t address, uint8_t val) {
    LR35902_write(((struct LR35902 *)c->emu)->userdata, address, val);
}

void altop_implied(instruction_t *i) {
    // Alter the opcode for implied instructions
    pick_at_random(&instructions_implied, &i->opcode);
}

void altop_rpimm16(instruction_t *i) {
    // Alter the opcode for implied instructions
    pick_at_random(&instructions_rpimm16, &i->opcode);
}

void rndoper_implied(rewrite_t *r, instruction_t *i) {
    // Randomise the operand for implied instructions;
    // nothing to do.
}

void rndoper_rpimm16(rewrite_t *r, instruction_t *i) {
    // Randomise either the register pair,
    // or the 2-byte immediate operand
    if (rand() & 0x01) {
        pick_at_random(&constants, &i->operand);
    } else {
        uint16_t rp = rand() & 0x38;
        i->opcode &= ~0x38;
        i->opcode |= rp;
    }
}

#define REGISTER_A ((struct LR35902 *)c->emu)->registers[7]
#define REGISTER_B ((struct LR35902 *)c->emu)->registers[0]
#define REGISTER_C ((struct LR35902 *)c->emu)->registers[1]

int live_in_a(stoc_t *c, decl_t *d, uint8_t **scram) {
    REGISTER_A = consume_scram(scram);
    return 0;
}

int live_in_b(stoc_t *c, decl_t *d, uint8_t **scram) {
    REGISTER_B = consume_scram(scram);
    return 0;
}

int live_in_c(stoc_t *c, decl_t *d, uint8_t **scram) {
    REGISTER_C = consume_scram(scram);
    return 0;
}

int live_out_a(stoc_t *c, decl_t *d, uint8_t **scram) {
    uint8_t s = consume_scram(scram);
    if (REGISTER_A == s) {
        return 0;
    } else {
        return 1;
    }
}

int setup_live_out_a(stoc_t *c, decl_t *d, uint8_t **scram) {
    output_scram(scram, REGISTER_A);
    return 0;
}

int live_out_b(stoc_t *c, decl_t *d, uint8_t **scram) {
    return REGISTER_B != consume_scram(scram);
}

int setup_live_out_b(stoc_t *c, decl_t *d, uint8_t **scram) {
    output_scram(scram, REGISTER_B);
    return 0;
}

int live_out_c(stoc_t *c, decl_t *d, uint8_t **scram) {
    return REGISTER_C != consume_scram(scram);
}

int setup_live_out_c(stoc_t *c, decl_t *d, uint8_t **scram) {
    output_scram(scram, REGISTER_C);
    return 0;
}

int live_in_stack(stoc_t *c, decl_t *d, uint8_t **scram) {
    fprintf(stderr, "unimplemented\n");
    exit(1);
}

int setup_live_in_stack(stoc_t *c, decl_t *d, uint8_t **scram) {
    fprintf(stderr, "unimplemented\n");
    exit(1);
}

int live_out_stack(stoc_t *c, decl_t *d, uint8_t **scram) {
    fprintf(stderr, "unimplemented\n");
    exit(1);
}

int setup_live_out_stack(stoc_t *c, decl_t *d, uint8_t **scram) {
    fprintf(stderr, "unimplemented\n");
    exit(1);
}

int start_decl(stoc_t *c, decl_t *d, uint8_t **scram) {
    for (int i = 0; i < 8; i++)
        ((struct LR35902 *)c->emu)->registers[i] = consume_scram(scram);
    // TODO: What to do about the stack pointer?
    return 0;
}

int setup_start_decl(stoc_t *c, decl_t *d, uint8_t **scram) {
    (*scram)[0] = rand();
    (*scram)[1] = rand();
    (*scram)[2] = rand();
    (*scram)[3] = rand();
    (*scram)[4] = rand();
    (*scram)[5] = rand();
    (*scram)[6] = rand();
    (*scram)[7] = rand();
    return start_decl(c, d, scram);
}

#define COMPUTEBUDGET 10000
int run_decl(stoc_t *c, decl_t *d, uint8_t **scram) {
    addr_t org = c->program.org;
    struct LR35902 *emu = c->emu;
    emu->PC = org;
    emu->cycles = 0;
    for (int i = 0; i < COMPUTEBUDGET; i++) {
        if (emu->PC == c->program.end) {
            c->clockticks = emu->cycles;
            return false; // the routine exited normally
        }
        if (emu->PC < org)
            return true; // program counter out of bounds
        if (emu->PC > c->program.end)
            return true; // program counter out of bounds
        LR35902_run(emu);
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
        memory_write(c, addr++, instr);
        if (opcode_length(instr) > 1)
            memory_write(c, addr++, r->instructions[i].operand & 0x00ff);
        if (opcode_length(instr) > 2)
            memory_write(c, addr++, r->instructions[i].operand >> 8);
    }
    r->blength = addr - r->org;
    r->end = r->org + r->blength;
}

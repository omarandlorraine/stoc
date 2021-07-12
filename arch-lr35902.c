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
    memset(c->emu, 0, sizeof(struct LR35902)); // shut valgrind up a bit
    ((struct LR35902 *)c->emu)->userdata = malloc(ADDRSPACE);
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

void altop_rimm8(instruction_t *i) {
    pick_at_random(&instructions_rimm8, &i->opcode);
}

void altop_rpimm16(instruction_t *i) {
    pick_at_random(&instructions_rpimm16, &i->opcode);
}

void rndoper_implied(rewrite_t *r, instruction_t *i) {
    // Randomise the operand for implied instructions;
    // nothing to do.
}

void rndoper_rimm8(rewrite_t *r, instruction_t *i) {
    // Randomise either the register,
    // or the 1-byte immediate operand
    if (rand() & 0x01) {
        pick_at_random(&constants, &i->operand);
    } else {
        pick_at_random(&instructions_rimm8, &i->opcode);
    }
}

void rndoper_rpimm16(rewrite_t *r, instruction_t *i) {
    // Randomise either the register pair,
    // or the 2-byte immediate operand
    if (rand() & 0x01) {
        pick_at_random(&constants, &i->operand);
    } else {
        pick_at_random(&instructions_rpimm16, &i->opcode);
    }
}

instrdata_t *instrdata(instruction_t *i) {
    int op = i->opcode & OPCODE_MASK;
    int pref = i->opcode & PREFIX_MASK;

    if (pref == PREFIX_NONE && instrdata_none[op].dis1)
        return &instrdata_none[op];

    if (pref == PREFIX_ED && instrdata_ed[op].dis1)
        return &instrdata_ed[op];

    if (pref == PREFIX_CB && instrdata_cb[op].dis1)
        return &instrdata_cb[op];

    if (pref == PREFIX_DD && instrdata_dd[op].dis1)
        return &instrdata_dd[op];

    if (pref == PREFIX_FD && instrdata_fd[op].dis1)
        return &instrdata_fd[op];

    if (pref == PREFIX_DDCB && instrdata_fdcb[op].dis1)
        return &instrdata_fdcb[op];

    if (pref == PREFIX_FDCB && instrdata_fdcb[op].dis1)
        return &instrdata_fdcb[op];

    fprintf(stderr, "No instruction data for instruction %04x\n", i->opcode);
    exit(1);
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
    uint8_t s = consume_scram(scram);
    if (REGISTER_B == s) {
        return 0;
    } else {
        return 1;
    }
}

int setup_live_out_b(stoc_t *c, decl_t *d, uint8_t **scram) {
    output_scram(scram, REGISTER_B);
    return 0;
}

int live_out_c(stoc_t *c, decl_t *d, uint8_t **scram) {
    uint8_t s = consume_scram(scram);
    if (REGISTER_C == s) {
        return 0;
    } else {
        return 1;
    }
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
    if (!strcmp(name, "b")) {
        d->fn = live_in_b;
        return;
    }
    if (!strcmp(name, "c")) {
        d->fn = live_in_c;
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
    if (!strcmp(name, "b")) {
        d->fn = live_out_b;
        d->setup = setup_live_out_b;
        return;
    }
    if (!strcmp(name, "c")) {
        d->fn = live_out_c;
        d->setup = setup_live_out_c;
        return;
    }
    fprintf(stderr, "Unknown register name \"%s\" in " __FILE__ "\n", name);
    exit(1);
}

void disasm_implied(instruction_t *i) { printf("\t%s\n", instrdata(i)->dis1); }

void disasm_rimm8(instruction_t *i) {
    printf("\t%s%02x%s\n", instrdata(i)->dis1, i->operand & 0xffU,
           instrdata(i)->dis2);
}

void disasm_rpimm16(instruction_t *i) {
    printf("\t%s%04x%s\n", instrdata(i)->dis1, i->operand, instrdata(i)->dis2);
}

void hexdump(stoc_t *c) {
    rewrite_t *r = &c->program;
    printf("; starting at $%04x\n", r->org);
    printf("; %d instructions\n", r->length);
    printf("; %d bytes\n; %d clockticks\n", r->blength, c->clockticks);
    for (int i = 0; i < r->length; i++) {
        instruction_t *instr = &r->instructions[i];
        instrdata(instr)->disasm(instr);
    }
    fprintf(stderr, "\n");
}

static pick_t zp_addresses;

void randomise_opcode(instruction_t *i) {
    pick_at_random(instrdata(i)->instrgroup, &i->opcode);
}

bool randomise_operand(rewrite_t *p, instruction_t *i) {
    instrdata(i)->rndoperand(p, i);
    return true;
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

int preflen(instruction_t *i) {
    uint16_t prefix = i->opcode & PREFIX_MASK;
    if (prefix == PREFIX_NONE)
        return 0;
    if (prefix == PREFIX_ED)
        return 1;
    if (prefix == PREFIX_CB)
        return 1;
    if (prefix == PREFIX_DD)
        return 1;
    if (prefix == PREFIX_FD)
        return 1;
    if (prefix == PREFIX_DDCB)
        return 2;
    if (prefix == PREFIX_FDCB)
        return 2;
    fprintf(stderr, "Parse issue, no preflen for PREFIX %02x\n", prefix);
    exit(1);
}

void install(stoc_t *c) {
    rewrite_t *r = &c->program;
    uint16_t addr = r->org;
    for (int i = 0; i < r->length; i++) {
        instruction_t *instr = &r->instructions[i];
        instr->address = addr;
        instrdata_t *id = instrdata(instr);

        if ((instr->opcode & PREFIX_MASK) == PREFIX_NONE) {
            // TODO: deal with prefixes
            memory_write(c, addr++, instr->opcode);
            if (id->operandlength >= 1)
                memory_write(c, addr++, instr->operand & 0x00ff);
            if (id->operandlength >= 2)
                memory_write(c, addr++, instr->operand >> 8);
        } else {
            fprintf(stderr,
                    "I don't know how to install this instruction %04x\n",
                    instr->opcode);
            exit(1);
        }
    }
    r->blength = addr - r->org;
    r->end = r->org + r->blength;
}

void mutate_opcode(instruction_t *i) {
    // TODO: Implement this properly
    pick_at_random(&all_instructions, &i->opcode);
}

bool exhsearch(stoc_t *reference, stoc_t *rewrite,
               bool (*continuation)(stoc_t *reference, stoc_t *rewrite),
               int i) {
    fprintf(stderr, "TODO: Actually implement exhsearch for lr35902\n");
    exit(1);
}

void read_prog(rewrite_t *r, uint8_t *raw, int length) {
    int offs = 0;
    int ins = 0;
    int address = r->org;

    for (;;) {
        if (offs > length)
            break;

        instruction_t *i = &(r->instructions[ins++]);
        i->address = address;

        uint8_t b = raw[offs++];
        uint8_t c = raw[offs];

        if (b == 0xed) {
            i->opcode = PREFIX_ED | c;
            offs++;
        }

        else if (b == 0xcb) {
            i->opcode = PREFIX_CB | c;
            offs++;
        }

        else if (b == 0xdd) {
            if (c == 0xcb) {
                offs++;
                i->operand = PREFIX_DDCB | raw[offs++];
                i->opcode = PREFIX_DDCB | raw[offs++];
            } else {
                i->opcode = PREFIX_DD | c;
                offs++;
            }
        }

        else if (b == 0xfd) {
            if (c == 0xcb) {
                offs++;
                i->operand = PREFIX_DDCB | raw[offs++];
                i->opcode = PREFIX_FDCB | raw[offs++];
            } else {
                i->opcode = PREFIX_FD | c;
                offs++;
            }
        }

        else {
            i->opcode = PREFIX_NONE | b;
        }

        if (instrdata(i)->operandlength == 1) {
            offs++;
            i->operand = raw[offs++];
        }

        if (instrdata(i)->operandlength == 2) {
            uint16_t l = raw[offs++];
            uint16_t h = raw[offs++];
            i->operand = (h << 8) | l;
        }
    }
    r->length = ins;
}

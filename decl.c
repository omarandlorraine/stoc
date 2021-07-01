#include "decl.h"
#include "arch.h"
#include "main.h"
#include "stdio.h"
#include "tests.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t consume_scram(uint8_t **scram) {
    uint8_t d = **scram;
    ++*scram;
    return d;
}

void output_scram(uint8_t **scram, uint8_t out) {
    **scram = out;
    ++*scram;
}

int setup_byte_in(stoc_t *c, decl_t *d, uint8_t **scram) {
    **scram = rand();
    return d->fn(c, d, scram);
}

int live_in_stack(stoc_t *c, decl_t *d, uint8_t **scram) {
    // TODO: This shouldn't be so 6502 specific
    mem_write(c, 0x0100 + c->s--, consume_scram(scram));
    return 0;
}

int setup_live_in_stack(stoc_t *c, decl_t *d, uint8_t **scram) {
    **scram = rand();
    return live_in_stack(c, d, scram);
}

int live_in_memory(stoc_t *c, decl_t *d, uint8_t **scram) {
    uint16_t addr = d->start;
    int len = d->length;
    while (len--)
        mem_write(c, addr++, consume_scram(scram));
    return 0;
}

int live_in_pointer(stoc_t *c, decl_t *d, uint8_t **scram) {

    // Construct a random pointer
    uint8_t l = consume_scram(scram);
    uint8_t h = consume_scram(scram);
    uint16_t addr = (h << 8) | l;

    // Write the pointer to memory
    mem_write(c, d->start, l);
    mem_write(c, d->start + 1, h);

    // Write "something" to the pointed-at location
    int len = d->length;
    while (len--)
        mem_write(c, addr++, consume_scram(scram));
    return 0;
}

int live_out_stack(stoc_t *c, decl_t *d, uint8_t **scram) {
    // TODO: This shouldn't be so 6502 specific
    return consume_scram(scram) != mem_read(c, 0x0100 + c->s--);
}

int setup_live_out_stack(stoc_t *c, decl_t *d, uint8_t **scram) {
    // TODO: This shouldn't be so 6502 specific
    output_scram(scram, mem_read(c, 0x0100 + c->s--));
    return 0;
}

int live_out_memory(stoc_t *c, decl_t *d, uint8_t **scram) {
    uint16_t addr = d->start;
    int len = d->length;
    while (len--)
        if (consume_scram(scram) != mem_read(c, addr++))
            return 1;
    return 0;
}

int setup_live_out_memory(stoc_t *c, decl_t *d, uint8_t **scram) {
    uint16_t addr = d->start;
    int len = d->length;
    while (len--)
        output_scram(scram, mem_read(c, addr++));
    return 0;
}

int live_out_pointer(stoc_t *c, decl_t *d, uint8_t **scram) {
    // TODO: This shouldn't assume little-endian 16-bit pointers.
    // This is going to bugger up support for other archs

    // Construct a random pointer
    uint8_t l = consume_scram(scram);
    uint8_t h = consume_scram(scram);
    uint16_t addr = (h << 8) | l;

    // Check the pointer
    if (mem_read(c, d->start) != l)
        return 1;
    if (mem_read(c, d->start + 1) != h)
        return 1;

    // Check the pointed-to data
    int len = d->length;
    while (len--)
        if (consume_scram(scram) != mem_read(c, addr++))
            return 1;
    return 0;
}

int setup_live_out_pointer(stoc_t *c, decl_t *d, uint8_t **scram) {
    // TODO: This shouldn't assume little-endian 16-bit pointers.
    // This is going to bugger up support for other archs

    // Reconstruct the pointer
    uint8_t l = mem_read(c, d->start);
    uint8_t h = mem_read(c, d->start + 1);
    uint16_t addr = (h << 8) | l;

    // Output the pointer
    output_scram(scram, l);
    output_scram(scram, h);

    // Output the pointed-to data
    int len = d->length;
    while (len--)
        output_scram(scram, mem_read(c, addr++));
    return 0;
}

int start_decl(stoc_t *c, decl_t *d, uint8_t **scram) {
    c->a = consume_scram(scram);
    c->x = consume_scram(scram);
    c->y = consume_scram(scram);
    c->s = consume_scram(scram);
    c->flags = consume_scram(scram);
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
    c->pc = org;
    for (int i = 0; i < COMPUTEBUDGET; i++) {
        if (c->pc == c->program.end)
            return NORMAL_EXIT;
        if (c->pc < org)
            return PC_OUT_OF_BOUNDS;
        if (c->pc > c->program.end)
            return PC_OUT_OF_BOUNDS;
        step(c);
    }
    return TOOK_TOO_LONG;
}

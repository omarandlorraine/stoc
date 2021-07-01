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

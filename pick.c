#include "decl.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define MAXADDRESSES 50

typedef struct {
    uint16_t vals[MAXADDRESSES];
    int count;
} pick_t;

static pick_t constants;
static pick_t zp_addresses;
static pick_t addresses;
static pick_t entrypoints;

static void insert(pick_t *pick, uint16_t val) {
    for (int offs = 0; offs < pick->count; offs++) {
        if (pick->vals[offs] == val)
            return;
    }
    pick->vals[pick->count++] = val;

    if (pick->count > MAXADDRESSES) {
        printf("Too many!");
        exit(1);
    }
}

static bool pick(pick_t *pick, uint16_t *out) {
    if (pick->count) {
        int p = rand() % pick->count;
        *out = pick->vals[p];
        return true;
    }
    return false;
}

bool random_address(uint16_t *out) { return pick(&addresses, out); }
bool random_zp_address(uint16_t *out) { return pick(&zp_addresses, out); }

bool random_constant(uint16_t *out) { return pick(&constants, out); }

void pickinit(decl_t *d) {
    constants.count = 0;
    addresses.count = 0;
    zp_addresses.count = 0;
    entrypoints.count = 0;

    // the most common constant values
    for (int i = -16; i < 17; i++)
        insert(&constants, i);

    for (uint8_t i = 1; i; i <<= 1)
        insert(&constants, i);

    while (d) {

        if (d->fn == live_out_memory && d->start < 256)
            insert(&zp_addresses, d->start);
        if (d->fn == live_in_memory && d->start < 256)
            insert(&zp_addresses, d->start);

        if (d->fn == live_out_memory)
            insert(&addresses, d->start);
        if (d->fn == live_in_memory)
            insert(&addresses, d->start);
        d = d->next;
    }
}

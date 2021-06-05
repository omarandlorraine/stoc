#include "pick.h"
#include "decl.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

pick_t constants;
pick_t addresses;
pick_t entrypoints;

void initialize_pick(pick_t *pick) { pick->count = 0; }

void pick_insert(pick_t *pick, uint16_t val) {
    for (int offs = 0; offs < pick->count; offs++) {
        if (pick->vals[offs] == val)
            return;
    }
    pick->vals[pick->count++] = val;

    if (pick->count > MAXPICKSIZE) {
        printf("Too many!");
        exit(1);
    }
}

bool pick_at_random(pick_t *pick, uint16_t *out) {
    if (pick->count) {
        int p = rand() % pick->count;
        *out = pick->vals[p];
        return true;
    }
    return false;
}

bool random_address(uint16_t *out) { return pick_at_random(&addresses, out); }

bool random_constant(uint16_t *out) { return pick_at_random(&constants, out); }

void iterator_init(pick_t *pick, iterator_t *iter) {
    iter->pick = pick;
    iter->current = 0;
}

bool pick_iterate(iterator_t *iter, uint16_t *out) {
    if (iter->current > iter->pick->count)
        return false;
    *out = iter->pick->vals[iter->current++];
    return true;
}

void pick_set_all_constants() {
    initialize_pick(&constants);
    for (int i = 0; i < 256; i++) {
        pick_insert(&constants, i);
    }
}

void pick_set_common_constants() {
    initialize_pick(&constants);
    // the most common constant values
    for (int i = -16; i < 17; i++)
        pick_insert(&constants, i);

    for (uint8_t i = 1; i; i <<= 1)
        pick_insert(&constants, i);
}

void pick_add_address(uint16_t addr) { pick_insert(&constants, addr); }

void pickinit() {
    pick_set_common_constants();
    initialize_pick(&addresses);
    initialize_pick(&entrypoints);
}

#pragma once
#include <stdbool.h>
#include <stdint.h>
#define MAXPICKSIZE 50

/**
 * @file
 * @brief Handling sets of values
 */

/** A set of values.
 *
 * This is intended to be a set, which we can pick a number at random from, or
 * which we can iterate over
 */
typedef struct {
    /// Number of members in the set
    int count;
    /// the members
    uint16_t vals[MAXPICKSIZE];
} pick_t;

/** An iterator
 *
 * This one is for iterating over a pick_t.
 */
typedef struct {
    /// Pointer to the pick_t object over which we're iterating
    pick_t *pick;
    /// Current offset
    int current;
} iterator_t;

extern pick_t constants;
extern pick_t addresses;

void iterator_init(pick_t *pick, iterator_t *iterator);
bool pick_iterate(iterator_t *iter, uint16_t *out);

void pick_insert(pick_t *pick, uint16_t val);
bool pick_at_random(pick_t *pick, uint16_t *out);

bool random_address(uint16_t *out);
bool random_constant(uint16_t *out);

void pickinit(void);

void initialize_pick(pick_t *pick);

void pick_set_all_constants();
void pick_set_common_constants();

#pragma once
#include <stdint.h>
#include <stdbool.h>
#define MAXPICKSIZE 50

typedef struct {
	int count;
	uint16_t vals[MAXPICKSIZE];
} pick_t;

typedef struct {
	pick_t * pick;
	int current;
} iterator_t;

extern pick_t constants;
extern pick_t addresses;

void iterator_init(pick_t * pick, iterator_t * iterator);
bool pick_iterate(iterator_t * iter, uint16_t * out);

void pick_insert(pick_t * pick, uint16_t val);
bool pick_at_random(pick_t * pick, uint16_t * out);

bool random_address(uint16_t * out);
bool random_constant(uint16_t * out);

void pickinit(void);

void initialize_pick(pick_t * pick);

void pick_set_all_constants();
void pick_set_common_constants();

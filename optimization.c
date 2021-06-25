#include "optimization.h"
#include "stoc.h"

static int (*measure)(stoc_t *c);

int optimize_size(stoc_t *c) { return c->program.blength; }

int optimize_speed(stoc_t *c) { return c->clockticks; }

int compare(stoc_t *a, stoc_t *b) { return measure(a) - measure(b); }

void set_optimization(int (*fn)(stoc_t *c)) { measure = fn; }

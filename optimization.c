#include "optimization.h"
#include "stoc.h"

static int (*measure)(context_t *c);

int optimize_size(context_t *c) { return c->program.blength; }

int optimize_speed(context_t *c) { return c->clockticks; }

int compare(context_t *a, context_t *b) { return measure(a) - measure(b); }

void set_optimization(int (*fn)(context_t *c)) { measure = fn; }

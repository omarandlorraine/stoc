#include "stoc.h"

int optimize_size(context_t * c);
int optimize_speed(context_t * c);

int compare(context_t * a, context_t * b);

void set_optimization(int (*fn)(context_t * c));

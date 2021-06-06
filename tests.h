#include "decl.h"
#include "stoc.h"

int equivalence(context_t *a, context_t *b);
void measure(context_t *c);

// The codes below need to be powers of two
#define NORMAL_EXIT 0
#define PC_OUT_OF_BOUNDS 1
#define TOOK_TOO_LONG 2

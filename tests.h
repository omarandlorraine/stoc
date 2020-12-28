#include "stoc.h"

int equivalence(context_t * a, context_t * b, int log);
void measure(context_t * reference, context_t * rewrite);

// The codes below need to be powers of two
#define NORMAL_EXIT 0
#define PC_OUT_OF_BOUNDS 1
#define TOOK_TOO_LONG 2

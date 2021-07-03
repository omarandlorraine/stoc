#include "decl.h"
#include "stoc.h"

int equivalence(stoc_t *a, stoc_t *b);
void measure(stoc_t *c);
void testcase_deinit();

// The codes below need to be powers of two
#define NORMAL_EXIT 0
#define PC_OUT_OF_BOUNDS 1
#define TOOK_TOO_LONG 2

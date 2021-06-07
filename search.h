#include "decl.h"
#include "stoc.h"

/**
 * @file
 * @brief search strategies
 */


/** Eliminate dead code
 *
 * Removes superfluous instructions from a program
 *
 * @param reference A context_t containing the rewrite you want to eliminate dead code from
 */
void deadcodeelim(context_t *reference);

/** Optimise
 *
 * Makes random mutations until an alternative rewrite is discovered that's functionally equivalent
 *
 * @param reference A context_t containing the rewrite you want to optimise
 */
void stoc_opt(context_t *reference);
void stoc_gen(context_t *reference);

void search_init();

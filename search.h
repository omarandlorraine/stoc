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
 * @param reference A stoc_t containing the rewrite you want to eliminate
 * dead code from
 */
void deadcodeelim(stoc_t *reference);

/** Optimise
 *
 * Makes random mutations until an alternative rewrite is discovered that's
 * functionally equivalent
 *
 * @param reference A stoc_t containing the rewrite you want to optimise
 */
void stoc_opt(stoc_t *reference);

/** Exhaustive search
 *
 * Does an exhaustive search of all programs upto a certain length, and prints
 * out the first one
 */
void stoc_exh(stoc_t *reference);

void search_init();
void cmdlin_intermediates();

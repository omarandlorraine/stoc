#include "stoc.h"

/**
 * @file
 * @brief Functions for measuring rewrites.
 */

/// Returns an integer representing the size of the rewrite
int optimize_size(context_t *c);

/// Returns an integer representing the speed of the rewrite
int optimize_speed(context_t *c);

/// Sets the optimization function
void set_optimization(int (*fn)(context_t *c));

/// Returns an integer representing a comparison of two rewrites, according to whichever function was selected by set_optimization
int compare(context_t *a, context_t *b);

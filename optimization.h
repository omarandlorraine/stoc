#include "stoc.h"

/**
 * @file
 * @brief Functions for measuring rewrites.
 */

/// Returns an integer representing the size of the rewrite
int optimize_size(stoc_t *c);

/// Returns an integer representing the speed of the rewrite
int optimize_speed(stoc_t *c);

/// Sets the optimization function
void set_optimization(int (*fn)(stoc_t *c));

/// Returns an integer representing a comparison of two rewrites, according to
/// whichever function was selected by set_optimization
int compare(stoc_t *a, stoc_t *b);

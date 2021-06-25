#include "decl.h"
#include "stoc.h"

/**
 * @file
 * @brief Parser for .stoc files
 */

/** Load a .stoc file
 *
 * @param filename The name of the file to load
 * @param reference A pointer to the stoc_t to load the file into
 */
void readfile(char *filename, stoc_t *reference);

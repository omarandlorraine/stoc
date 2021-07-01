#include "decl.h"

/**
 * @file
 * @brief Architecture-specific things go here.
 */


/** Write a value to the emulator's address space
 *
 * @param c Which emulated machine we're writing to the address space of
 * @param address The address to write to
 * @param val The value to write
 */
void mem_write(stoc_t *c, addr_t address, data_t val);

/** Read a value in the emulator's address space
 *
 * @param c Which emulated machine we're writing to the address space of
 * @param address The address to read from
 * @return the value read from the memory location
 */
uint8_t mem_read(stoc_t *c, addr_t address);

/** Single-step the emulator
 *
 * Execute a single instruction
 *
 * @param c Which emulated machine
 */
void step(stoc_t *c);


// TODO: doxygen for these functions
void register_in_name(decl_t *d, char *name);
void register_out_name(decl_t *d, char *name);
void hexdump(stoc_t *c);

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

/** Disassemble the file
 *
 * Outputs the rewrite, plus miscellaneous measurements, to stdout.
 */
void hexdump(stoc_t *r);

// TODO: doxygen for these functions
void register_in_name(decl_t *d, char *name);
void register_out_name(decl_t *d, char *name);
int live_in_stack(stoc_t *c, decl_t *d, uint8_t **scram);
int setup_live_in_stack(stoc_t *c, decl_t *d, uint8_t **scram);

void hexdump(stoc_t *c);
void randomise_opcode(instruction_t *i);
bool randomise_operand(rewrite_t *p, instruction_t *i);
void mutate_opcode(instruction_t *i);
bool exhsearch(stoc_t *reference, stoc_t *rewrite,  bool (*continuation)(stoc_t * reference, stoc_t * rewrite), int i);
void archsearch_init();
void install(stoc_t *c);
void arch_init(stoc_t *c);
void arch_deinit(stoc_t *c);

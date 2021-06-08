/**
 * @file
 * @brief Interface to the emulator
 */

/** Write a value to the emulator's address space
 *
 * @param c Which emulated machine we're writing to the address space of
 * @param address The address to write to
 * @param val The value to write
 */
void mem_write(context_t *c, addr_t address, data_t val);

/** Read a value in the emulator's address space
 *
 * @param c Which emulated machine we're writing to the address space of
 * @param address The address to read from
 * @return the value read from the memory location
 */
uint8_t mem_read(context_t *c, addr_t address);

/** Single-step the emulator
 *
 * Execute a single instruction
 *
 * @param c Which emulated machine
 */
void step(context_t *c);

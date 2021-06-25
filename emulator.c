#include "emulator.h"
#include "stoc.h"

uint8_t mem_read(stoc_t *c, uint16_t address) { return c->mem[address]; }

void mem_write(stoc_t *c, uint16_t address, uint8_t val) {
    c->mem[address] = val;
}

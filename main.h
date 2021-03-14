#include <stdint.h>
#include "stoc.h"
uint8_t mem_read(context_t * c, addr_t address);
void mem_write(context_t * c, addr_t address, data_t val);
void hexdump(context_t * r);

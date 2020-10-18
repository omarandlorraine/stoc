#include "fake6502.h"

uint16_t ea_immediate(Context65 c) {
	return c.pc + 1;
}

uint8_t get_accumulator(Context65 c) {
	return c.a;
}

uint8_t get_immediate(Context65 c) {
	
}

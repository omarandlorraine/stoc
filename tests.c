#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include "stoc.h"
#include "tests.h"
#include "reg.h"
#include "main.h"
#include "emulator.h"
#define MAXITER 10000		// How many times are we going to try throwing random numbers at the routine?
#define COMPUTEBUDGET 10000 // For how many cycles may the routine run?

void install(context_t * c) {
	rewrite_t * r = &c->program;
	uint16_t addr = r->org;
	for(int i = 0; i < r->length; i++) {
		uint8_t instr = r->instructions[i].opcode;
		r->instructions[i].address = addr;
		mem_write(c, addr++, instr);
		if(opcode_length(instr) > 1) mem_write(c, addr++, r->instructions[i].operand & 0x00ff);
		if(opcode_length(instr) > 2) mem_write(c, addr++, r->instructions[i].operand >> 8);
		r->end = addr;
	}
}

int run(context_t * c) {
	addr_t org = c->program.org;
	c->pc = org;
	for(int i = 0; i < COMPUTEBUDGET; i++) {
		if(c->pc == c->program.end) return NORMAL_EXIT;
		if(c->pc < org) return PC_OUT_OF_BOUNDS;
		if(c->pc > c->program.end) return PC_OUT_OF_BOUNDS;
		step(c);
	}
	return TOOK_TOO_LONG;
}

int equivalence(context_t * reference, context_t * rewrite, int log) {
	install(reference);
	install(rewrite);
	for(int i = 0; i < MAXITER; i++) {
		uint8_t a = rand();
		uint8_t x = rand();
		uint8_t y = rand();
		uint8_t f = rand();
		
		reg_rand_in(reference);
		reg_rand_out(reference);
		reg_rand_in(rewrite);
		reg_rand_out(rewrite);
		rewrite->a = reference->a = a;
		rewrite->x = reference->x = x;
		rewrite->y = reference->y = y;
		rewrite->flags = reference->flags = f;

		int code = run(rewrite);
		if(code) {
			return 0;
		}
		code = run(reference);
		if(code) {
			return 0;
		}
		if(reg_cmp_out(reference, rewrite)) {
			if(log--) {
				printf("a = %02x, x = %02x, y = %02x\n", a, x, y);
			} else {
				return 0;
			}
		}
	}
	return 1;
}

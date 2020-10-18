#include <inttypes.h>
#include <stdlib.h>
#include "stoc.h"
#include "emulators/fake6502.h"
#include "tests.h"
#include "reg.h"
#define MAXITER 10000		// How many times are we going to try throwing random numbers at the routine?
#define COMPUTEBUDGET 10000 // For how many cycles may the routine run?

extern Context65 reference;
extern Context65 rewrite;
extern int org;

void install(Context65 * ctx, rewrite_t * r) {
	uint16_t addr = org;
	for(int i = 0; i < r->length; i++) {
		uint8_t instr = r->instructions[i].opcode;
		write6502(ctx, addr++, instr);
		if(opcode_length(instr) > 1) write6502(ctx, addr++, r->instructions[i].operand & 0x00ff);
		if(opcode_length(instr) > 2) write6502(ctx, addr++, r->instructions[i].operand >> 8);
	}
	r->end = addr;
}

int run(Context65 * c, rewrite_t * r) {
	c->pc = org;
	for(int i = 0; i < COMPUTEBUDGET; i++) {
		if(c->pc == r->end) return NORMAL_EXIT;
		if(c->pc < org) return PC_OUT_OF_BOUNDS;
		if(c->pc > r->end) return PC_OUT_OF_BOUNDS;
		step6502(c);
	}
	return TOOK_TOO_LONG;
}

int equivalence(rewrite_t * a, rewrite_t * b) {
	install(&reference, a);
	install(&rewrite, b);

	for(int i = 0; i < MAXITER; i++) {
		reg_rand_in(&reference);
		reg_rand_out(&reference);
		reg_rand_in(&rewrite);
		reg_rand_out(&rewrite);
		if(run(&rewrite, b)) return 0;
		if(run(&reference, a)) return 0;
		if(reg_cmp_out(&reference, &rewrite)) return 0;
	}
	return 1;
}
